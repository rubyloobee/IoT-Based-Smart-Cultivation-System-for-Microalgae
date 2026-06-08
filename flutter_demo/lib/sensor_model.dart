import 'package:cloud_firestore/cloud_firestore.dart';

class SensorData {
  final String systemId;
  final double temperature;
  final double pH;
  final double waterLevel;
  final double conductivity;
  final double lightIntensity;
  final double turbidity;
  final double color;
  final String timestamp;

  SensorData({
    required this.systemId,
    this.temperature = 0.0,
    this.pH = 0.0,
    this.waterLevel = 0.0,
    this.conductivity = 0.0,
    this.lightIntensity = 0.0,
    this.turbidity = 0.0,
    this.color = 0.0,
    this.timestamp = '',
  });

  factory SensorData.fromFirestore(DocumentSnapshot doc, String systemId) {
    final data = doc.data() as Map<String, dynamic>? ?? {};

    double parse(String key) {
      final value = data[key];
      if (value == null) return 0.0;
      return double.tryParse(value.toString()) ?? 0.0;
    }

    return SensorData(
      systemId: systemId,
      temperature: parse('temperature_C'),
      pH: parse('pH_value'),
      waterLevel: parse('water_level_pct'),
      lightIntensity: parse('light_intensity_lux'),
      conductivity: parse('EC_mscm'),
      turbidity: parse('absorbance_RGB'),
      color: parse('absorbance_turbidity'),
      timestamp: doc.id,
    );
  }
}
