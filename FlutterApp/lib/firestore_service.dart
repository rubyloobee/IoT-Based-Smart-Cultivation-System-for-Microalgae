import 'package:cloud_firestore/cloud_firestore.dart';
import 'control_settings.dart';
import 'sensor_model.dart';
import 'ntp_sync.dart';

class FirestoreService {
  final FirebaseFirestore _db = FirebaseFirestore.instance;

  /// Updates the light intensity and other control setpoints in Firestore
  Future<void> updateSystemControls(
      String systemId, ControlSettings settings) async {
    final String docId = systemId.toLowerCase().replaceAll(' ', '_');
    try {
      await Future.wait<void>([
        // 1. Actuators
        _db.collection('system_controls').doc(docId).update({
          'target_light_intensity': settings.lightIntensity,
          'target_light_duration': settings.lightDuration,
          'target_water_level': settings.targetWaterLevel,
          'target_stirring_speed': settings.stirringSpeed,
          'timestamp': FieldValue.serverTimestamp(),
          'harvest_auto_enabled': settings.harvestAutoEnabled,
          'harvest_min_time': settings.harvestMinTime,
          'harvest_rgb_threshold': settings.harvestRgbThreshold,
          'harvest_turbidity_threshold': settings.harvestTurbidityThreshold,
          'nutrient_auto_enabled': settings.nutrientAutoEnabled,
          'nutrient_ec_threshold': settings.nutrientEcThreshold,
          'nutrient_min_time': settings.nutrientMinTime,
        }),

        // 2. Logging Intervals
        _db.collection('log_interval').doc(docId).set({
          'primary_log_interval': settings.primaryLogInterval,
          'sampling_log_interval': settings.samplingLogInterval,
          'timestamp': FieldValue.serverTimestamp(),
        }, SetOptions(merge: true)),

        // 3. Data Threholds & FCM Token
        _db.collection('data_thresholds').doc(docId).set({
          'temp_min': settings.tempMin,
          'temp_max': settings.tempMax,
          'ph_min': settings.phMin,
          'ph_max': settings.phMax,
          'ec_min': settings.ecMin,
          'ec_max': settings.ecMax,
          'fcmToken': settings.fcmToken,
          'last_updated': FieldValue.serverTimestamp(),
        }, SetOptions(merge: true)),
      ]);

      print("Firestore: Actuators and Logging synced for $systemId");
    } catch (e) {
      print("Firestore Error: $e");
      rethrow;
    }
  }

  Future<ControlSettings?> fetchSystemSettings(String systemId) async {
    final String docId = systemId.toLowerCase().replaceAll(' ', '_');
    try {
      // 1. Fetch all three documents at once
      final snapshots = await Future.wait<DocumentSnapshot>([
        _db.collection('system_controls').doc(docId).get(),
        _db.collection('log_interval').doc(docId).get(),
        _db.collection('data_thresholds').doc(docId).get(),
      ]);

      // 2. Extract data (return null if documents don't exist yet)
      final controls = snapshots[0].data() as Map<String, dynamic>?;
      final logs = snapshots[1].data() as Map<String, dynamic>?;
      final thresholds = snapshots[2].data() as Map<String, dynamic>?;

      if (controls == null || logs == null || thresholds == null) return null;

      // 3. Map Firestore types (usually num/int) to Dart doubles
      return ControlSettings(
        lightIntensity: (controls['target_light_intensity'] as num).toDouble(),
        lightDuration:
            (controls['target_light_duration'] as num).toDouble() / 3600,
        targetWaterLevel: (controls['target_water_level'] as num).toDouble(),
        stirringSpeed: (controls['target_stirring_speed'] as num).toDouble(),
        primaryLogInterval: (logs['primary_log_interval'] as num).toDouble(),
        samplingLogInterval: (logs['sampling_log_interval'] as num).toDouble(),
        tempMin: (thresholds['temp_min'] as num).toDouble(),
        tempMax: (thresholds['temp_max'] as num).toDouble(),
        phMin: (thresholds['ph_min'] as num).toDouble(),
        phMax: (thresholds['ph_max'] as num).toDouble(),
        ecMin: (thresholds['ec_min'] as num).toDouble(),
        ecMax: (thresholds['ec_max'] as num).toDouble(),
        fcmToken: thresholds['fcmToken'] ?? "initial_token_placeholder",
        harvestAutoEnabled:
            (controls['harvest_auto_enabled'] as bool? ?? false),
        harvestIsRequested:
            (controls['harvest_is_requested'] as bool? ?? false),
        harvestMinTime: (controls['harvest_min_time'] as num).toDouble() / 3600,
        harvestRgbThreshold:
            (controls['harvest_rgb_threshold'] as num? ?? 0).toDouble(),
        harvestTurbidityThreshold:
            (controls['harvest_turbidity_threshold'] as num? ?? 0).toDouble(),
        nutrientAutoEnabled:
            (controls['nutrient_auto_enabled'] as bool? ?? false),
        nutrientIsRequested:
            (controls['nutrient_is_requested'] as bool? ?? false),
        nutrientMinTime:
            (controls['nutrient_min_time'] as num).toDouble() / 3600,
        nutrientEcThreshold:
            (controls['nutrient_ec_threshold'] as num? ?? 0).toDouble(),
      );
    } catch (e) {
      print("Fetch error: $e");
      return null;
    }
  }

  // Fetch the most recent activity logs
  Stream<QuerySnapshot> getSystemActivity(String systemId) {
    final String docId = systemId.toLowerCase().replaceAll(' ', '_');

    return _db
        .collection('system_activity')
        .doc(docId)
        .collection('log')
        .orderBy(FieldPath.documentId, descending: true)
        .limit(10)
        .snapshots();
  }

  // Fetch most recent sensor data
  Stream<SensorData> getLiveSensorData(String systemId, String tankType) {
    // Determine the collection name (main_tank_data or sampling_tank_data)
    final String collectionName = (tankType.toLowerCase() == 'main')
        ? 'main_tank_data'
        : 'sampling_tank_data';

    final String docId = systemId.toLowerCase().replaceAll(' ', '_');

    return _db
        .collection(collectionName)
        .doc(docId)
        .collection('logs')
        .orderBy(FieldPath.documentId, descending: true)
        .limit(1)
        .snapshots()
        .map((snapshot) {
      if (snapshot.docs.isNotEmpty) {
        print('--- $tankType Sensor Data Renewed ---');
        int unixMs = TimeSyncService.getCurrentTime();
        print("Flutter Time: $unixMs");
        return SensorData.fromFirestore(snapshot.docs.first, systemId);
      }
      return SensorData(systemId: systemId);
    });
  }

  Stream<List<SensorData>> getHistoricalSensorData(
      String systemId, String tankType) {
    final String collectionName = (tankType.toLowerCase() == 'main')
        ? 'main_tank_data'
        : 'sampling_tank_data';
    final String docId = systemId.toLowerCase().replaceAll(' ', '_');

    return _db
        .collection(collectionName)
        .doc(docId)
        .collection('logs')
        .orderBy(FieldPath.documentId, descending: true)
        .limit(10)
        .snapshots()
        .map((snapshot) => snapshot.docs
            .map((doc) => SensorData.fromFirestore(doc, systemId))
            .toList()
            .reversed
            .toList());
  }
}
