import 'package:flutter/material.dart';
import 'package:cloud_firestore/cloud_firestore.dart';
import 'control_settings.dart';
import 'notification_service.dart';
import 'firestore_service.dart';
import 'sensor_model.dart';
import 'ntp_sync.dart';

// Create and manage accompanying state object _MonitoringScreenState
class MonitoringScreen extends StatefulWidget {
  final String systemId;
  const MonitoringScreen({required this.systemId, super.key});

  @override
  State<MonitoringScreen> createState() => _MonitoringScreenState();
}

class _MonitoringScreenState extends State<MonitoringScreen> {
  @override
  void initState() {
    super.initState();
  }

  @override
  void dispose() {
    super.dispose();
  }

  @override
  // Called to build the UI whenever needed
  Widget build(BuildContext context) {
    return Scaffold(
      body: MonitoringDashboard(systemId: widget.systemId),
    );
  }
}

class MonitoringDashboard extends StatefulWidget {
  final String systemId;

  const MonitoringDashboard({
    required this.systemId,
    super.key,
  });

  @override
  State<MonitoringDashboard> createState() => _MonitoringDashboardState();
}

// StatelessWidget: widget's appearance and behaviour are fixed once it is created
class _MonitoringDashboardState extends State<MonitoringDashboard> {
  final ControlSettingsManager _settingsManager = ControlSettingsManager();

  final FirestoreService _firestoreService = FirestoreService();

  late DocumentReference _cameraDoc;

  late Stream<QuerySnapshot> _activityStream;

  int? _captureStartTime;

  @override
  void initState() {
    super.initState();
    _initializeCameraRef();
    _syncNotificationToken();
    _activityStream = _firestoreService.getSystemActivity(widget.systemId);
  }

  // If user switches between "System 1" and "System 2"
  @override
  void didUpdateWidget(MonitoringDashboard oldWidget) {
    super.didUpdateWidget(oldWidget);
    if (widget.systemId != oldWidget.systemId) {
      _initializeCameraRef();
      _syncNotificationToken();
      setState(() {
        _activityStream = _firestoreService.getSystemActivity(widget.systemId);
      });
    }
  }

  void _initializeCameraRef() {
    // Converts "System 1" to "system_1" to match Firestore document ID
    String docId = widget.systemId.toLowerCase().replaceAll(' ', '_');
    _cameraDoc = FirebaseFirestore.instance.collection('camera').doc(docId);
  }

  void _syncNotificationToken() {
    NotificationService().initialize(widget.systemId);
  }

  // Triggers the Raspberry Pi by updating Firestore request
  Future<void> _triggerCapture() async {
    try {
      _captureStartTime = TimeSyncService.getCurrentTime();
      print("--- Camera Capture Requested ---");
      print("Start Time: $_captureStartTime");
      await _cameraDoc.update({'isCaptureRequested': true});
    } catch (e) {
      _captureStartTime = null; // Reset on error
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text('Error: ${e.toString()}')),
      );
    }
  }

  // Determine if a sensor is active/present for the current system.
  bool _isSensorActive(String sensorName, SensorData data) {
    if (widget.systemId == 'System 1')
      return true; // All sensors active on System 1

    // For System 2, only temperature is active
    if (sensorName == 'Temperature') {
      return data.temperature != 0.0;
    }
    // All other sensors are inactive in System 2
    return false;
  }

  // Helper method to build the individual sensor cards
  Widget _buildSensorCard({
    required String title,
    required String value,
    required IconData icon,
    required Color color,
    bool isCritical = false,
    required bool isActive,
  }) {
    // If sensor is not active, set the critical flag and display red card
    final displayTitle = isActive ? title : '$title (Inactive)';
    final displayValue = isActive ? value : 'N/A';
    final cardColor = isCritical ? Colors.red.shade800 : color;
    final valueStyle = TextStyle(
      fontSize: 28,
      fontWeight: FontWeight.w900,
      color: isCritical ? Colors.white : Colors.teal.shade900,
    );

    final iconColor = isCritical ? Colors.white : Colors.white;

    final titleStyle = TextStyle(
      fontSize: 16,
      fontWeight: FontWeight.bold,
      color: isCritical ? Colors.white : Colors.grey.shade700,
    );

    const double iconSize = 18;
    const double avatarRadius = 14;

    return Card(
      // Colour of card is white when not critical
      color: isCritical ? cardColor : Colors.white,
      elevation: 5,
      shape: RoundedRectangleBorder(
        borderRadius: BorderRadius.circular(15),
        side: BorderSide(
          color: isCritical ? Colors.red.shade900 : Colors.teal.shade200,
          width: 2,
        ),
      ),
      child: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          mainAxisAlignment: MainAxisAlignment.spaceBetween,
          children: <Widget>[
            Row(
              children: [
                CircleAvatar(
                  backgroundColor: cardColor,
                  radius: avatarRadius,
                  child: Icon(icon, color: iconColor, size: iconSize),
                ),
                const SizedBox(width: 8),
                Expanded(
                  child: Text(
                    displayTitle,
                    style: titleStyle,
                    overflow: TextOverflow.ellipsis,
                  ),
                ),
              ],
            ),
            Align(
              alignment: Alignment.bottomRight,
              child: Text(displayValue, style: valueStyle),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildCameraSection() {
    // System 2: Camera is not available
    if (widget.systemId == 'System 2') {
      return Padding(
        padding: const EdgeInsets.symmetric(vertical: 20.0),
        child: Center(
          child: Text(
            'Camera Feature Not Available for ${widget.systemId}',
            style: TextStyle(
                fontSize: 16, fontStyle: FontStyle.italic, color: Colors.grey),
          ),
        ),
      );
    }

    // System 1 logic: Camera is available
    // Creates live connection to Firestore document
    return StreamBuilder<DocumentSnapshot>(
      stream: _cameraDoc.snapshots(),
      // Rebuilds UI everytime the document is updated by Pi
      builder: (context, snapshot) {
        if (snapshot.hasError) return const Text("Camera sync error");
        if (!snapshot.hasData) return const LinearProgressIndicator();

        var data = snapshot.data!.data() as Map<String, dynamic>?;
        bool isBusy = data?['isCaptureRequested'] ?? false;
        String? imageUrl = data?['last_image_url'];

        // Calculate camera latency time
        if (!isBusy && _captureStartTime != null && imageUrl != null) {
          int endTime = TimeSyncService.getCurrentTime();
          int totalLatency = endTime - _captureStartTime!;

          print("--- Camera Image Received ---");
          print("End Time: $endTime");
          print(
              "Total Camera Latency: ${totalLatency / 1000} seconds ($totalLatency ms)");

          _captureStartTime = null;
        }

        // Extract the timestamp and convert it to a readable String
        final dynamic timestampRaw = data?['last_capture_time'];
        String timeLabel = "Never captured";

        if (timestampRaw != null && timestampRaw is Timestamp) {
          DateTime dt = timestampRaw.toDate().toLocal();
          timeLabel =
              "${dt.day}/${dt.month}/${dt.year} at ${dt.hour}:${dt.minute.toString().padLeft(2, '0')}";
        }

        return Column(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            const SizedBox(height: 16),
            Text(
              'Live System Image',
              style: TextStyle(
                  fontSize: 18,
                  fontWeight: FontWeight.bold,
                  color: Colors.teal.shade700),
            ),
            const Divider(height: 20, thickness: 1),

            // Image Container
            Container(
              height: 220,
              decoration: BoxDecoration(
                color: Colors.grey.shade200,
                border: Border.all(color: Colors.teal.shade100, width: 2),
              ),
              child: isBusy
                  ? const Center(
                      child: Column(
                        mainAxisAlignment: MainAxisAlignment.center,
                        children: [
                          CircularProgressIndicator(color: Colors.teal),
                          SizedBox(height: 12),
                          Text("Pi is capturing image...",
                              style: TextStyle(
                                  color: Colors.teal,
                                  fontWeight: FontWeight.bold)),
                        ],
                      ),
                    )
                  : (imageUrl != null
                      ? Image.network(
                          imageUrl,
                          key: ValueKey(imageUrl),
                          fit: BoxFit.cover,
                          // Checks image downloading progress
                          loadingBuilder: (context, child, progress) {
                            if (progress == null) return child;
                            return const Center(
                                child: CircularProgressIndicator());
                          },
                          // eg. phone loses internet mid-download, image was deleted from the bucket
                          errorBuilder: (context, error, stack) => const Icon(
                              Icons.broken_image,
                              size: 50,
                              color: Colors.grey),
                        )
                      : const Center(child: Text("No image available"))),
            ),
            const SizedBox(height: 12),

            // Timestamp Label
            Center(
              child: Text(
                "Last captured: $timeLabel",
                style: TextStyle(
                  fontSize: 14,
                  fontStyle: FontStyle.italic,
                  color: Colors.grey.shade600,
                ),
              ),
            ),

            const SizedBox(height: 12),

            // Renew Button
            ElevatedButton.icon(
              onPressed: isBusy ? null : _triggerCapture,
              icon: Icon(isBusy ? Icons.hourglass_top : Icons.camera_alt),
              label: Text(isBusy ? 'Capturing...' : 'Renew System Image'),
              style: ElevatedButton.styleFrom(
                minimumSize: const Size(double.infinity, 50),
                backgroundColor: Colors.teal,
                foregroundColor: Colors.white,
                shape: RoundedRectangleBorder(
                    borderRadius: BorderRadius.circular(10)),
              ),
            ),
          ],
        );
      },
    );
  }

  Widget _buildActivityLogItem(QueryDocumentSnapshot doc) {
    final data = doc.data() as Map<String, dynamic>;
    final String type = data['type'] ?? 'Unknown';
    final String trigger = data['trigger'] ?? 'auto';
    final Map<String, dynamic>? values =
        data['value_at_event'] as Map<String, dynamic>?;

    // Use the document ID as the display time
    final String displayTime = doc.id;

    return Card(
      key: ValueKey(doc.id),
      margin: const EdgeInsets.symmetric(vertical: 8, horizontal: 4),
      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(10)),
      child: ListTile(
        leading: Icon(
          type == 'Harvest' ? Icons.eco : Icons.opacity,
          color: Colors.teal,
        ),
        title: Text('$type ($trigger)',
            style: const TextStyle(fontSize: 16, fontWeight: FontWeight.bold)),
        subtitle: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            if (values != null) ...[
              // Conditional check for Harvest
              if (type == 'Harvest')
                Text(
                    'RGB: ${values['rgb']} abs | Turb: ${values['turbidity']} abs',
                    style: const TextStyle(fontSize: 16)),

              // Conditional check for Nutrient Delivery
              if (type == 'Nutrient Delivery')
                Text('EC: ${values['ec']} mS/cm',
                    style: const TextStyle(fontSize: 16)),
            ],
            Text(displayTime,
                style: const TextStyle(fontSize: 14, color: Colors.grey)),
          ],
        ),
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    // Get current control settings
    final controlSettings = _settingsManager.getSettings(widget.systemId);

    // 1. First builder listens to Main Tank
    return StreamBuilder<SensorData>(
      stream: _firestoreService.getLiveSensorData(widget.systemId, 'main'),
      builder: (context, mainSnapshot) {
        if (mainSnapshot.connectionState == ConnectionState.waiting) {
          return const Center(child: CircularProgressIndicator());
        }

        // 2. Inside the first builder, start a second builder for Sampling Tank
        return StreamBuilder<SensorData>(
          stream:
              _firestoreService.getLiveSensorData(widget.systemId, 'sampling'),
          builder: (context, samplingSnapshot) {
            if (samplingSnapshot.connectionState == ConnectionState.waiting) {
              return const Center(child: CircularProgressIndicator());
            }

            // 3. Once both snapshots have data, build the UI
            if (mainSnapshot.hasData && samplingSnapshot.hasData) {
              final mainData = mainSnapshot.data!;
              final samplingData = samplingSnapshot.data!;

              // Ensure the dashboard is fully responsive and usable on different screen sizes
              // Enable vertical scrolling if content exceed height of device screen
              return SingleChildScrollView(
                padding: const EdgeInsets.all(16.0),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Padding(
                      padding: const EdgeInsets.only(top: 8.0, bottom: 20.0),
                      child: Row(
                        children: [
                          Container(
                            height: 24,
                            width: 3.5,
                            decoration: BoxDecoration(
                              color: Colors.teal.shade700,
                              borderRadius: BorderRadius.circular(2),
                            ),
                          ),
                          const SizedBox(width: 12),
                          // The System ID text
                          Text(
                            widget.systemId,
                            style: TextStyle(
                              fontSize: 20,
                              fontWeight: FontWeight.w600,
                              color: Colors.grey.shade800,
                              letterSpacing: 0.5,
                            ),
                          ),
                        ],
                      ),
                    ),

                    // --- MAIN TANK DATA ---
                    Row(
                      mainAxisAlignment: MainAxisAlignment.spaceBetween,
                      children: [
                        Text(
                          'Main Tank Data',
                          style: TextStyle(
                              fontSize: 18,
                              fontWeight: FontWeight.bold,
                              color: Colors.teal.shade800),
                        ),
                        // Show Main Tank Timestamp
                        Text(
                          mainData.timestamp, // Pulls from doc.id
                          style: TextStyle(
                              fontSize: 16, color: Colors.grey.shade600),
                        ),
                      ],
                    ),
                    const Divider(thickness: 1),

                    _buildSensorCard(
                      title: 'Temperature',
                      // Live SensorData object extracted from StreamBuilder snapshot
                      value: '${mainData.temperature.toStringAsFixed(2)} °C',
                      icon: Icons.thermostat_outlined,
                      color: Colors.red.shade300,
                      isActive: _isSensorActive('Temperature', mainData),
                      isCritical:
                          mainData.temperature > controlSettings.tempMax ||
                              mainData.temperature < controlSettings.tempMin,
                    ),

                    const SizedBox(height: 16),

                    _buildSensorCard(
                      title: 'pH Level',
                      value: mainData.pH.toStringAsFixed(2),
                      icon: Icons.science_outlined,
                      color: Colors.green.shade300,
                      isActive: _isSensorActive('pH Level', mainData),
                      isCritical: _isSensorActive('pH Level', mainData) &&
                          (mainData.pH > controlSettings.phMax ||
                              mainData.pH < controlSettings.phMin),
                    ),
                    const SizedBox(height: 16),

                    _buildSensorCard(
                      title: 'Water Level',
                      value: '${mainData.waterLevel.toStringAsFixed(2)}%',
                      icon: Icons.water_drop_outlined,
                      color: Colors.blue.shade300,
                      isActive: _isSensorActive('Water Level', mainData),
                      isCritical: mainData.waterLevel < 0,
                    ),
                    const SizedBox(height: 16),

                    _buildSensorCard(
                      title: 'Light Intensity',
                      value:
                          '${mainData.lightIntensity.toStringAsFixed(2)} lux',
                      icon: Icons.lightbulb_outline,
                      color: Colors.amber.shade300,
                      isActive: _isSensorActive('Light Intensity', mainData),
                    ),
                    const SizedBox(height: 16),

                    // --- SAMPLING TANK SENSORS ---
                    Row(
                      mainAxisAlignment: MainAxisAlignment.spaceBetween,
                      children: [
                        Text(
                          'Sampling Tank Data',
                          style: TextStyle(
                              fontSize: 18,
                              fontWeight: FontWeight.bold,
                              color: Colors.teal.shade800),
                        ),
                        // Show Sampling Tank Timestamp
                        Text(
                          samplingData.timestamp,
                          style: TextStyle(
                              fontSize: 16, color: Colors.grey.shade600),
                        ),
                      ],
                    ),
                    const Divider(thickness: 1),

                    _buildSensorCard(
                      title: 'Conductivity',
                      value:
                          '${samplingData.conductivity.toStringAsFixed(2)} mS/cm',
                      icon: Icons.scatter_plot_outlined,
                      color: Colors.purple.shade300,
                      isActive: _isSensorActive('Conductivity', samplingData),
                      isCritical: _isSensorActive(
                              'Conductivity', samplingData) &&
                          (samplingData.conductivity > controlSettings.ecMax ||
                              samplingData.conductivity <
                                  controlSettings.ecMin),
                    ),

                    _buildSensorCard(
                      title: 'Turbidity',
                      value: '${samplingData.turbidity.toStringAsFixed(2)} abs',
                      icon: Icons.opacity_outlined,
                      color: Colors.brown.shade300,
                      isActive: _isSensorActive('Turbidity', samplingData),
                    ),
                    const SizedBox(height: 16),

                    _buildSensorCard(
                      title: 'Colour Density',
                      value: '${samplingData.color.toStringAsFixed(2)} abs',
                      icon: Icons.color_lens_outlined,
                      color: Colors.lime.shade300,
                      isActive: _isSensorActive('Colour Density', samplingData),
                    ),
                    const SizedBox(height: 16),

                    _buildCameraSection(),
                    const SizedBox(height: 32),

                    Text(
                      'System Activity History',
                      style: TextStyle(
                          fontSize: 18,
                          fontWeight: FontWeight.bold,
                          color: Colors.teal.shade700),
                    ),
                    const Divider(),

                    StreamBuilder<QuerySnapshot>(
                      stream: _activityStream,
                      builder: (context, snapshot) {
                        if (snapshot.hasError)
                          return Text("Error: ${snapshot.error}");
                        if (snapshot.connectionState ==
                            ConnectionState.waiting) {
                          return const Center(
                              child: CircularProgressIndicator());
                        }

                        final logs = snapshot.data?.docs ?? [];

                        if (logs.isEmpty) {
                          return const Padding(
                            padding: EdgeInsets.symmetric(vertical: 20),
                            child:
                                Text("No activity logs found for this system."),
                          );
                        }

                        return Column(
                          children: logs
                              .map((doc) => _buildActivityLogItem(doc))
                              .toList(),
                        );
                      },
                    ),
                  ],
                ),
              );
            }
            return const Center(child: Text('No sensor data available.'));
          },
        );
      },
    );
  }
}
