import 'package:flutter/material.dart';
import 'control_settings.dart';
import 'firestore_service.dart';
import 'package:cloud_firestore/cloud_firestore.dart';
import 'ntp_sync.dart';

// Control Screen as a StatefulWidget to manage input state
class ControlScreen extends StatefulWidget {
  final String systemId;
  const ControlScreen({required this.systemId, super.key});

  @override
  State<ControlScreen> createState() => _ControlScreenState();
}

class _ControlScreenState extends State<ControlScreen> {
  bool _isLoading = true;
  final ControlSettingsManager _manager = ControlSettingsManager();

  // --- Current System Settings State ---
  late double _targetWaterLevel;
  late double _stirringSpeed;
  late double _lightIntensity;
  late double _lightDuration;
  late double _primaryLogInterval;
  late double _samplingLogInterval;
  late bool _harvestAutoEnabled;
  late bool _harvestIsRequested;
  late double _harvestMinTime;
  late double _harvestRgbThreshold;
  late double _harvestTurbidityThreshold;
  late bool _nutrientAutoEnabled;
  late bool _nutrientIsRequested;
  late double _nutrientMinTime;
  late double _nutrientEcThreshold;

  // --- Alert Threshold Text Controllers ---
  late final TextEditingController _tempMinController;
  late final TextEditingController _tempMaxController;
  late final TextEditingController _phMinController;
  late final TextEditingController _phMaxController;
  late final TextEditingController _ecMinController;
  late final TextEditingController _ecMaxController;

  // Key for validation
  final _formKey = GlobalKey<FormState>();

  // --- Conversion Constants ---
  // 1, 2, 3 hours
  final Map<int, double> _primaryIntervals = {
    1: 3600.0, // 1 hour
    2: 7200.0, // 2 hours
    3: 10800.0, // 3 hours
  };
  // 1, 2, 3 times per day (24h, 12h, 8h)
  final Map<int, double> _samplingIntervals = {
    1: 86400.0, // 24 hours (Once Daily)
    2: 43200.0, // 12 hours (Twice Daily)
    3: 28800.0, // 8 hours (Three Times Daily)
  };

  // Reinitialises all local state from the manager's persistent data
  void _initializeStateFromManager() {
    final settings = _manager.getSettings(widget.systemId);

    _targetWaterLevel = settings.targetWaterLevel;
    _stirringSpeed = settings.stirringSpeed;
    _lightIntensity = settings.lightIntensity;
    _lightDuration = settings.lightDuration;
    _primaryLogInterval = settings.primaryLogInterval;
    _samplingLogInterval = settings.samplingLogInterval;
    _harvestAutoEnabled = settings.harvestAutoEnabled;
    _harvestIsRequested = settings.harvestIsRequested;
    _harvestMinTime = settings.harvestMinTime;
    _harvestRgbThreshold = settings.harvestRgbThreshold;
    _harvestTurbidityThreshold = settings.harvestTurbidityThreshold;
    _nutrientAutoEnabled = settings.nutrientAutoEnabled;
    _nutrientIsRequested = settings.nutrientIsRequested;
    _nutrientMinTime = settings.nutrientMinTime;
    _nutrientEcThreshold = settings.nutrientEcThreshold;

    _tempMinController.text = settings.tempMin.toStringAsFixed(1);
    _tempMaxController.text = settings.tempMax.toStringAsFixed(1);
    _phMinController.text = settings.phMin.toStringAsFixed(1);
    _phMaxController.text = settings.phMax.toStringAsFixed(1);
    _ecMinController.text = settings.ecMin.toStringAsFixed(1);
    _ecMaxController.text = settings.ecMax.toStringAsFixed(1);
  }

  @override
  void initState() {
    super.initState();

    _tempMinController = TextEditingController();
    _tempMaxController = TextEditingController();
    _phMinController = TextEditingController();
    _phMaxController = TextEditingController();
    _ecMinController = TextEditingController();
    _ecMaxController = TextEditingController();

    _initializeStateFromManager();

    _loadCloudData();
  }

  // Ensures the state updates if the parent widget rebuilds with a different systemId
  @override
  void didUpdateWidget(ControlScreen oldWidget) {
    super.didUpdateWidget(oldWidget);

    if (widget.systemId != oldWidget.systemId) {
      setState(() {
        _initializeStateFromManager();
      });
    }
  }

  @override
  void dispose() {
    _tempMinController.dispose();
    _tempMaxController.dispose();
    _phMinController.dispose();
    _phMaxController.dispose();
    _ecMinController.dispose();
    _ecMaxController.dispose();
    super.dispose();
  }

  Future<void> _loadCloudData() async {
    setState(() => _isLoading = true);

    final cloudSettings =
        await _firestoreService.fetchSystemSettings(widget.systemId);

    if (cloudSettings != null) {
      _manager.updateSettings(widget.systemId, cloudSettings);
    }

    setState(() {
      _initializeStateFromManager();
      _isLoading = false;
    });
  }

  // --- Conversion Logic ---
  String _formatPrimaryInterval(double interval) {
    final hours = (interval / 3600).toInt();
    return '$hours Hour${hours > 1 ? 's' : ''}';
  }

  String _formatSamplingInterval(double interval) {
    if (interval >= 86400) return 'Once Daily';
    if (interval >= 43200) return 'Twice Daily';
    if (interval >= 28800) return '3 Times Daily';
    return 'Custom';
  }

  String _formatInterval(double interval) {
    return '${interval.toInt()} ';
  }

  // Gets the current discrete slider step value (1, 2, or 3) from the stored seconds
  double _getPrimarySliderValue() {
    return _primaryIntervals.entries
        .firstWhere((entry) => entry.value == _primaryLogInterval,
            orElse: () => const MapEntry(
                2, 7200.0) // Default to 2 hours if value is custom/missing
            )
        .key
        .toDouble();
  }

  double _getSamplingSliderValue() {
    return _samplingIntervals.entries
        .firstWhere((entry) => entry.value == _samplingLogInterval,
            orElse: () => const MapEntry(2, 43200.0))
        .key
        .toDouble();
  }

  final FirestoreService _firestoreService = FirestoreService();

  // --- Action Logic ---
  void _saveSettings() async {
    // Validate all form fields
    if (_formKey.currentState!.validate()) {
      final phoneTime = TimeSyncService.getCurrentTime();
      print('--- Actuator Settings Updated ---');
      print('Flutter Phone Time:  $phoneTime');
      double roundToOneDecimal(double value) => (value * 10).round() / 10.0;

      final currentSettings = _manager.getSettings(widget.systemId);

      // Create the ControlSettings object from current state
      final newSettings = ControlSettings(
        targetWaterLevel: roundToOneDecimal(_targetWaterLevel),
        stirringSpeed: roundToOneDecimal(_stirringSpeed),
        lightIntensity: roundToOneDecimal(_lightIntensity),
        lightDuration:
            roundToOneDecimal(_lightDuration * 3600), // Convert to seconds
        primaryLogInterval: roundToOneDecimal(_primaryLogInterval),
        samplingLogInterval: roundToOneDecimal(_samplingLogInterval),

        fcmToken: currentSettings.fcmToken,

        // Parse current text field values
        tempMin: double.tryParse(_tempMinController.text) ?? 0.0,
        tempMax: double.tryParse(_tempMaxController.text) ?? 0.0,
        phMin: double.tryParse(_phMinController.text) ?? 0.0,
        phMax: double.tryParse(_phMaxController.text) ?? 0.0,
        ecMin: double.tryParse(_ecMinController.text) ?? 0.0,
        ecMax: double.tryParse(_ecMaxController.text) ?? 0.0,

        // Harvesting and nutrient delivery
        harvestAutoEnabled: _harvestAutoEnabled,

        harvestMinTime:
            roundToOneDecimal(_harvestMinTime * 3600), // Convert to seconds
        harvestRgbThreshold: roundToOneDecimal(_harvestRgbThreshold),
        harvestTurbidityThreshold:
            roundToOneDecimal(_harvestTurbidityThreshold),
        nutrientAutoEnabled: _nutrientAutoEnabled,

        nutrientMinTime:
            roundToOneDecimal(_nutrientMinTime * 3600), // Convert to seconds
        nutrientEcThreshold: roundToOneDecimal(_nutrientEcThreshold),

        harvestIsRequested: false,
        nutrientIsRequested: false,
      );

      _manager.updateSettings(widget.systemId, newSettings);

      // Upload to Firestore
      try {
        await _firestoreService.updateSystemControls(
            widget.systemId, newSettings);
        _showConfirmationDialog();
      } catch (e) {
        if (!mounted) return;
        // Show an error snackbar if the cloud upload fails
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(content: Text('Failed to sync with cloud: $e')),
        );
      }
    }
  }

  void _showConfirmationDialog() {
    showDialog(
      context: context,
      builder: (context) {
        return AlertDialog(
          title: const Text('Settings Saved!'),
          content: const Text(
              'Your new control parameters have been applied to the system.'),
          actions: <Widget>[
            TextButton(
              onPressed: () => Navigator.of(context).pop(),
              child: const Text('OK'),
            ),
          ],
        );
      },
    );
  }

  // Helper to show a confirmation dialog before manual actions
  Future<void> _showManualActionConfirmation(String actionType) async {
    final bool isHarvest = actionType == 'harvest';

    final bool? confirmed = await showDialog<bool>(
      context: context,
      builder: (context) => AlertDialog(
        title: Text('Confirm ${isHarvest ? 'Harvest' : 'Nutrient Delivery'}'),
        content: Text(
            'Are you sure? This will activate the system pumps immediately.'),
        actions: [
          TextButton(
            onPressed: () => Navigator.pop(context, false),
            child: const Text('Cancel'),
          ),
          ElevatedButton(
            onPressed: () => Navigator.pop(context, true),
            style: ElevatedButton.styleFrom(
              backgroundColor:
                  isHarvest ? Colors.orange.shade700 : Colors.blue.shade700,
              foregroundColor: Colors.white,
            ),
            child: const Text('Confirm Action'),
          ),
        ],
      ),
    );

    if (confirmed == true) {
      final phoneTime = TimeSyncService.getCurrentTime();
      print('--- Manual Command Triggered ---');
      print('Flutter Phone Time:  $phoneTime');
      _sendManualCommand(actionType);
    }
  }

  void _sendManualCommand(String type) async {
    final String docId = widget.systemId.toLowerCase().replaceAll(' ', '_');
    final bool isHarvest = type == 'harvest';

    try {
      // Use .update to target only specific fields
      await FirebaseFirestore.instance
          .collection('system_controls')
          .doc(docId)
          .update({
        if (isHarvest) 'harvest_is_requested': true,
        if (!isHarvest) 'nutrient_is_requested': true,
        'timestamp': FieldValue.serverTimestamp(),
      });

      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
            content:
                Text('Manual ${isHarvest ? 'Harvest' : 'Dosing'} Triggered!')),
      );
    } catch (e) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text('Error: $e'), backgroundColor: Colors.red),
      );
    }
  }

  // --- UI Helper Widgets ---

  // Standardized card layout for all control groups
  Widget _buildControlCard({
    required String title,
    required IconData icon,
    required Widget content,
  }) {
    return Card(
      elevation: 6,
      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(15)),
      margin: const EdgeInsets.only(bottom: 24),
      child: Padding(
        padding: const EdgeInsets.all(20.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Row(
              children: [
                Icon(icon, color: Colors.teal.shade700, size: 28),
                const SizedBox(width: 12),
                Text(
                  title,
                  style: TextStyle(
                    fontSize: 20,
                    fontWeight: FontWeight.bold,
                    color: Colors.teal.shade900,
                  ),
                ),
              ],
            ),
            const Divider(height: 25, thickness: 1),
            content,
          ],
        ),
      ),
    );
  }

  // Specific helper for slider inputs
  Widget _buildSliderControl({
    required String label,
    required double value,
    required double min,
    required double max,
    required int divisions,
    required String unit,
    required Function(double) onChanged,
    String? displayValueOverride,
  }) {
    final displayValue = displayValueOverride ?? value.toStringAsFixed(0);

    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Row(
          mainAxisAlignment: MainAxisAlignment.spaceBetween,
          children: [
            Text(
              label,
              style: TextStyle(fontSize: 16, color: Colors.grey.shade700),
            ),
            Text(
              unit.isNotEmpty ? '$displayValue$unit' : displayValue,
              style: TextStyle(
                  fontSize: 20,
                  fontWeight: FontWeight.bold,
                  color: Colors.teal.shade800),
            ),
          ],
        ),
        Slider(
          value: value,
          min: min,
          max: max,
          divisions: divisions,
          label: value.toStringAsFixed(0) + unit,
          activeColor: Colors.teal,
          inactiveColor: Colors.teal.shade100,
          onChanged: onChanged,
        ),
      ],
    );
  }

  Widget _buildLoggingContent() {
    // Slow logging interval
    final currentPrimaryLabel = _formatPrimaryInterval(_primaryLogInterval);
    final currentSamplingLabel = _formatSamplingInterval(_samplingLogInterval);

    final primarySliderValue = _getPrimarySliderValue();
    final samplingSliderValue = _getSamplingSliderValue();

    return Column(
      children: [
        // Main tank data (every 1-3 hours)
        _buildSliderControl(
          label: 'Main Tank',
          // Slow logging interval
          //value: primarySliderValue,
          //min: 1,
          //max: 3,
          //divisions: 2,
          //unit: '',
          //displayValueOverride: currentPrimaryLabel,
          // Fast logging interval
          value: _primaryLogInterval,
          min: 30,
          max: 300,
          divisions: 9,
          unit: 's',
          displayValueOverride: _formatInterval(_primaryLogInterval),
          onChanged: (newValue) {
            setState(() {
              // Slow logging interval
              //final newInterval = _primaryIntervals[newValue.toInt()];
              //if (newInterval != null) {
              //  _primaryLogInterval = newInterval;
              //}
              // Fast logging interval
              _primaryLogInterval = newValue;
            });
          },
        ),
        Text(
          "Sensors: Temperature, pH, Water Level, Light",
          style: TextStyle(
              fontSize: 14,
              color: Colors.grey.shade500,
              fontStyle: FontStyle.italic),
        ),

        const SizedBox(height: 20),
        const Divider(),
        const SizedBox(height: 10),

        // Sampling tank data (1-3 times daily)
        _buildSliderControl(
          label: 'Sampling Tank',
          // Slow logging interval
          //value: samplingSliderValue,
          //min: 1,
          //max: 3,
          //divisions: 2,
          //unit: '',
          //displayValueOverride: currentSamplingLabel,
          // Fast logging interval
          value: _samplingLogInterval,
          min: 30,
          max: 300,
          divisions: 9,
          unit: 's',
          displayValueOverride: _formatInterval(_samplingLogInterval),
          onChanged: (newValue) {
            setState(() {
              // Slow logging interval
              final newInterval = _samplingIntervals[newValue.toInt()];
              if (newInterval != null) {
                _samplingLogInterval = newInterval;
              }
              // Fast logging interval
              // _samplingLogInterval = newValue;
            });
          },
        ),
        Text(
          "Sensor: Conductivity, Turbidity, Colour Density",
          style: TextStyle(
              fontSize: 14,
              color: Colors.grey.shade500,
              fontStyle: FontStyle.italic),
        ),
      ],
    );
  }

  Widget _buildLightingContent() {
    // Calculate Dark hours based on Light hours
    final int lightHours = _lightDuration.toInt();
    final int darkHours = 24 - lightHours;

    return Column(
      children: [
        // 1. Intensity Slider
        _buildSliderControl(
          label: 'LED Intensity',
          value: _lightIntensity,
          min: 0,
          max: 500,
          divisions: 10,
          unit: ' Lux',
          onChanged: (newValue) {
            setState(() {
              _lightIntensity = newValue;
            });
          },
        ),

        const SizedBox(height: 20),
        const Divider(),
        const SizedBox(height: 10),

        // 2. Photoperiod Slider
        Row(
          mainAxisAlignment: MainAxisAlignment.spaceBetween,
          children: [
            Text('Photoperiod (Daily)',
                style: TextStyle(fontSize: 16, color: Colors.grey.shade700)),
            RichText(
              text: TextSpan(
                style: TextStyle(fontSize: 18, fontWeight: FontWeight.bold),
                children: [
                  TextSpan(
                      text: '${lightHours}h ON',
                      style: TextStyle(color: Colors.teal.shade800)),
                  const TextSpan(
                      text: ' / ', style: TextStyle(color: Colors.grey)),
                  TextSpan(
                      text: '${darkHours}h OFF',
                      style: TextStyle(color: Colors.teal.shade900)),
                ],
              ),
            ),
          ],
        ),
        Slider(
          value: _lightDuration,
          min: 0,
          max: 24,
          divisions: 24,
          label: '$lightHours hours Light',
          activeColor: Colors.teal,
          inactiveColor: Colors.teal.shade100,
          onChanged: (newValue) {
            setState(() {
              _lightDuration = newValue;
            });
          },
        ),
        Text(
          "Adjusts the daily Light:Dark cycle",
          style: TextStyle(
              fontSize: 14,
              color: Colors.grey.shade500,
              fontStyle: FontStyle.italic),
        ),
      ],
    );
  }

  // Specific helper for min/max threshold inputs (pH, Temperature, EC)
  Widget _buildThresholdControl({
    required String label,
    required TextEditingController minController,
    required TextEditingController maxController,
    required String unit,
    required double globalMin,
    required double globalMax,
  }) {
    // Custom validator for all numeric fields
    String? validator(String? value, bool isMinField) {
      if (value == null || value.isEmpty) {
        return 'Required';
      }
      final numValue = double.tryParse(value);
      if (numValue == null) {
        return 'Must be a valid number';
      }

      // 1. Enforce physical/safe limits
      if (numValue < globalMin || numValue > globalMax) {
        final minStr = globalMin.toStringAsFixed(1);
        final maxStr = globalMax.toStringAsFixed(1);

        return 'Range: $minStr - $maxStr';
      }

      // 2. Enforce Min < Max relationship
      final otherController = isMinField ? maxController : minController;
      final otherValue = double.tryParse(otherController.text);

      if (otherValue != null) {
        if (isMinField && numValue >= otherValue) {
          return 'Must be < than Max';
        }
        if (!isMinField && numValue <= otherValue) {
          return 'Must be > than Min';
        }
      }

      return null;
    }

    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text(
          '$label Alert Thresholds ($unit)',
          style: TextStyle(fontSize: 16, color: Colors.grey.shade700),
        ),
        const SizedBox(height: 10),
        Row(
          children: [
            // Minimum Input
            Expanded(
              child: TextFormField(
                controller: minController,
                keyboardType: TextInputType.number,
                decoration: InputDecoration(
                  labelText: 'Minimum',
                  suffixText: unit,
                  border: const OutlineInputBorder(
                      borderRadius: BorderRadius.all(Radius.circular(10))),
                  isDense: true,
                ),
                validator: (value) => validator(value, true),
                onChanged: (_) {
                  if (_formKey.currentState != null) {
                    _formKey.currentState!.validate();
                  }
                },
              ),
            ),
            const SizedBox(width: 16),
            // Maximum Input
            Expanded(
              child: TextFormField(
                controller: maxController,
                keyboardType: TextInputType.number,
                decoration: InputDecoration(
                  labelText: 'Maximum',
                  suffixText: unit,
                  border: const OutlineInputBorder(
                      borderRadius: BorderRadius.all(Radius.circular(10))),
                  isDense: true,
                ),
                validator: (value) => validator(value, false),
                onChanged: (_) {
                  if (_formKey.currentState != null) {
                    _formKey.currentState!.validate();
                  }
                },
              ),
            ),
          ],
        ),
      ],
    );
  }

  Widget _buildSwitchControl({
    required String title,
    required String subtitle,
    required bool value,
    required Function(bool) onChanged,
  }) {
    return SwitchListTile(
      title: Text(
        title,
        style: const TextStyle(fontSize: 16, fontWeight: FontWeight.bold),
      ),
      subtitle: Text(
        subtitle,
        style: TextStyle(fontSize: 14, color: Colors.grey.shade600),
      ),
      value: value,
      activeThumbColor: Colors.teal,
      onChanged: onChanged,
      contentPadding: EdgeInsets.zero, // Aligns with the card edges
    );
  }

  @override
  Widget build(BuildContext context) {
    return Container(
      color: Colors.teal.shade50,
      child: _isLoading
          ? const Center(child: CircularProgressIndicator())
          : SingleChildScrollView(
              padding: const EdgeInsets.all(16.0),
              child: Form(
                key: _formKey,
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

                    _buildControlCard(
                      title: 'Data Logging Frequency',
                      icon: Icons.access_time_outlined,
                      content: _buildLoggingContent(),
                    ),

                    _buildControlCard(
                      title: 'Lighting',
                      icon: Icons.lightbulb_outlined,
                      content: _buildLightingContent(),
                    ),

                    // --- 1. Water Level Control ---
                    _buildControlCard(
                      title: 'Target Water Level',
                      icon: Icons.water_drop_outlined,
                      content: _buildSliderControl(
                        label: 'Set desired water level:',
                        value: _targetWaterLevel,
                        min: 20,
                        max: 80,
                        divisions: 5,
                        unit: '%',
                        onChanged: (newValue) {
                          setState(() {
                            _targetWaterLevel = newValue;
                          });
                        },
                      ),
                    ),

                    // --- 2. Motor Stirring Speed Control ---
                    _buildControlCard(
                      title: 'Motor Stirring Speed',
                      icon: Icons.rotate_right_outlined,
                      content: _buildSliderControl(
                        label: 'Set motor stirring speed:',
                        value: _stirringSpeed,
                        min: 50,
                        max: 200,
                        divisions: 3,
                        unit: ' RPM',
                        onChanged: (newValue) {
                          setState(() {
                            _stirringSpeed = newValue;
                          });
                        },
                      ),
                    ),

                    // --- 3. Temperature Alert Thresholds ---
                    _buildControlCard(
                      title: 'Temperature Alert Settings',
                      icon: Icons.thermostat_outlined,
                      content: _buildThresholdControl(
                        label: 'Temperature',
                        minController: _tempMinController,
                        maxController: _tempMaxController,
                        unit: '°C',
                        // Set temperature limits
                        globalMin: 10.0,
                        globalMax: 40.0,
                      ),
                    ),

                    // --- 4. pH Alert Thresholds ---
                    _buildControlCard(
                      title: 'pH Alert Settings',
                      icon: Icons.science_outlined,
                      content: _buildThresholdControl(
                        label: 'pH Level',
                        minController: _phMinController,
                        maxController: _phMaxController,
                        unit: 'pH',
                        // Set pH limits
                        globalMin: 5.0,
                        globalMax: 10.0,
                      ),
                    ),

                    // --- 5. EC Alert Thresholds ---
                    _buildControlCard(
                      title: 'Conductivity Alert Settings',
                      icon: Icons.scatter_plot_outlined,
                      content: _buildThresholdControl(
                        label: 'Conductivity',
                        minController: _ecMinController,
                        maxController: _ecMaxController,
                        unit: 'ms/cm',
                        // Set EC limits
                        globalMin: 0.2,
                        globalMax: 3.0,
                      ),
                    ),

                    _buildControlCard(
                      title: 'Harvesting Control',
                      icon: Icons.agriculture_outlined,
                      content: Column(
                        children: [
                          _buildSwitchControl(
                            title: 'Enable Auto-Harvest',
                            subtitle:
                                'Harvesting triggers when density meets either Turbidity or RGB criteria',
                            value: _harvestAutoEnabled,
                            onChanged: (bool val) =>
                                setState(() => _harvestAutoEnabled = val),
                          ),
                          const SizedBox(height: 10),
                          _buildSliderControl(
                            label: 'Turbidity Threshold',
                            value: _harvestTurbidityThreshold,
                            min: 2,
                            max: 5,
                            divisions: 15,
                            unit: ' abs',
                            displayValueOverride:
                                _harvestTurbidityThreshold.toStringAsFixed(1),
                            onChanged: (val) => setState(
                                () => _harvestTurbidityThreshold = val),
                          ),
                          _buildSliderControl(
                            label: 'RGB Density Threshold',
                            value: _harvestRgbThreshold,
                            min: 2,
                            max: 5,
                            divisions: 15,
                            unit: ' abs',
                            displayValueOverride:
                                _harvestRgbThreshold.toStringAsFixed(1),
                            onChanged: (val) =>
                                setState(() => _harvestRgbThreshold = val),
                          ),
                          _buildSliderControl(
                            label: 'Min Time Between Harvests',
                            value: _harvestMinTime,
                            min: 12,
                            max: 24,
                            divisions: 12,
                            unit: ' hours',
                            onChanged: (val) =>
                                setState(() => _harvestMinTime = val),
                          ),
                          const Divider(),
                          const SizedBox(height: 10),
                          SizedBox(
                            width: double.infinity,
                            child: ElevatedButton.icon(
                              onPressed: () =>
                                  _showManualActionConfirmation('harvest'),
                              icon: const Icon(Icons.handshake_outlined),
                              label: const Text('Manual Harvest Now'),
                              style: ElevatedButton.styleFrom(
                                backgroundColor: Colors.orange.shade700,
                                foregroundColor: Colors.white,
                              ),
                            ),
                          ),
                        ],
                      ),
                    ),

                    _buildControlCard(
                      title: 'Nutrient Delivery Control',
                      icon: Icons.biotech_outlined,
                      content: Column(
                        children: [
                          _buildSwitchControl(
                            title: 'Enable Auto-Nutrient Delivery',
                            subtitle:
                                'Dose nutrients when EC falls below threshold',
                            value: _nutrientAutoEnabled,
                            onChanged: (bool val) =>
                                setState(() => _nutrientAutoEnabled = val),
                          ),
                          const SizedBox(height: 10),
                          _buildSliderControl(
                            label: 'EC Threshold (Minimum)',
                            value: _nutrientEcThreshold,
                            min: 1.5,
                            max: 3.0,
                            divisions: 15, // 0.1 increments
                            unit: ' mS/cm',
                            displayValueOverride:
                                _nutrientEcThreshold.toStringAsFixed(1),
                            onChanged: (val) =>
                                setState(() => _nutrientEcThreshold = val),
                          ),
                          _buildSliderControl(
                            label: 'Min Time Between Doses',
                            value: _nutrientMinTime,
                            min: 4,
                            max: 12,
                            divisions: 8,
                            unit: ' hours',
                            onChanged: (val) =>
                                setState(() => _nutrientMinTime = val),
                          ),
                          const Divider(),
                          const SizedBox(height: 10),
                          SizedBox(
                            width: double.infinity,
                            child: ElevatedButton.icon(
                              onPressed: () =>
                                  _showManualActionConfirmation('nutrient'),
                              icon: const Icon(Icons.add_chart),
                              label: const Text('Manual Nutrient Delivery Now'),
                              style: ElevatedButton.styleFrom(
                                backgroundColor: Colors.blue.shade700,
                                foregroundColor: Colors.white,
                              ),
                            ),
                          ),
                        ],
                      ),
                    ),

                    const SizedBox(height: 16),

                    // --- Save Button ---
                    Center(
                      child: ElevatedButton.icon(
                        onPressed: _saveSettings,
                        icon: const Icon(Icons.save),
                        label: const Text('Save Settings to System',
                            style: TextStyle(fontSize: 16)),
                        style: ElevatedButton.styleFrom(
                          backgroundColor: Colors.teal.shade700,
                          foregroundColor: Colors.white,
                          padding: const EdgeInsets.symmetric(
                              horizontal: 30, vertical: 15),
                          shape: RoundedRectangleBorder(
                              borderRadius: BorderRadius.circular(12)),
                          elevation: 8,
                        ),
                      ),
                    ),
                    const SizedBox(height: 40),
                  ],
                ),
              ),
            ),
    );
  }
}
