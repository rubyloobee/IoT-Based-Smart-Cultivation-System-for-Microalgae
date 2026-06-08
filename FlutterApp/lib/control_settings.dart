import 'package:flutter/material.dart';

// 1. Data Model for Control Settings
class ControlSettings {
  final double targetWaterLevel;
  final double stirringSpeed;
  final double lightIntensity;
  final double lightDuration;

  final double primaryLogInterval;
  final double samplingLogInterval;

  // Alert threshold
  final double tempMin;
  final double tempMax;
  final double phMin;
  final double phMax;
  final double ecMin;
  final double ecMax;
  final String fcmToken;

  // Harvest and Nutrient Delivery
  final bool harvestAutoEnabled;
  final bool harvestIsRequested;
  final double harvestMinTime;
  final double harvestRgbThreshold;
  final double harvestTurbidityThreshold;
  final bool nutrientAutoEnabled;
  final double nutrientEcThreshold;
  final bool nutrientIsRequested;
  final double nutrientMinTime;

  ControlSettings({
    required this.targetWaterLevel,
    required this.stirringSpeed,
    required this.lightIntensity,
    required this.lightDuration,
    required this.primaryLogInterval,
    required this.samplingLogInterval,
    required this.tempMin,
    required this.tempMax,
    required this.phMin,
    required this.phMax,
    required this.ecMin,
    required this.ecMax,
    required this.fcmToken,
    required this.harvestAutoEnabled,
    required this.harvestIsRequested,
    required this.harvestMinTime,
    required this.harvestRgbThreshold,
    required this.harvestTurbidityThreshold,
    required this.nutrientAutoEnabled,
    required this.nutrientEcThreshold,
    required this.nutrientIsRequested,
    required this.nutrientMinTime,
  });

  // Factory method to create a new instance from an existing one
  ControlSettings copyWith({
    double? targetWaterLevel,
    double? stirringSpeed,
    double? lightIntensity,
    double? lightDuration,
    double? primaryLogInterval,
    double? samplingLogInterval,
    double? tempMin,
    double? tempMax,
    double? phMin,
    double? phMax,
    double? ecMin,
    double? ecMax,
    String? fcmToken,
    bool? harvestAutoEnabled,
    bool? harvestIsRequested,
    double? harvestMinTime,
    double? harvestRgbThreshold,
    double? harvestTurbidityThreshold,
    bool? nutrientAutoEnabled,
    double? nutrientEcThreshold,
    bool? nutrientIsRequested,
    double? nutrientMinTime,
  }) {
    return ControlSettings(
      targetWaterLevel: targetWaterLevel ?? this.targetWaterLevel,
      stirringSpeed: stirringSpeed ?? this.stirringSpeed,
      lightIntensity: lightIntensity ?? this.lightIntensity,
      lightDuration: lightDuration ?? this.lightDuration,
      primaryLogInterval: primaryLogInterval ?? this.primaryLogInterval,
      samplingLogInterval: samplingLogInterval ?? this.samplingLogInterval,
      tempMin: tempMin ?? this.tempMin,
      tempMax: tempMax ?? this.tempMax,
      phMin: phMin ?? this.phMin,
      phMax: phMax ?? this.phMax,
      ecMin: ecMin ?? this.ecMin,
      ecMax: ecMax ?? this.ecMax,
      fcmToken: fcmToken ?? this.fcmToken,
      harvestAutoEnabled: harvestAutoEnabled ?? this.harvestAutoEnabled,
      harvestIsRequested: harvestIsRequested ?? this.harvestIsRequested,
      harvestMinTime: harvestMinTime ?? this.harvestMinTime,
      harvestRgbThreshold: harvestRgbThreshold ?? this.harvestRgbThreshold,
      harvestTurbidityThreshold:
          harvestTurbidityThreshold ?? this.harvestTurbidityThreshold,
      nutrientAutoEnabled: nutrientAutoEnabled ?? this.nutrientAutoEnabled,
      nutrientEcThreshold: nutrientEcThreshold ?? this.nutrientEcThreshold,
      nutrientIsRequested: nutrientIsRequested ?? this.nutrientIsRequested,
      nutrientMinTime: nutrientMinTime ?? this.nutrientMinTime,
    );
  }

  @override
  String toString() {
    return '--- Control Settings Manager State ---\n'
        'Primary Log Interval: ${primaryLogInterval.toStringAsFixed(0)} s\n'
        'Sampling Log Interval: ${samplingLogInterval.toStringAsFixed(0)} s\n'
        'LED intensity: ${lightIntensity.toStringAsFixed(0)}\n'
        'Photoperiod: ${lightDuration.toStringAsFixed(0)} seconds (${(lightDuration / 3600).toStringAsFixed(1)}h)\n'
        'Water Level Target: ${targetWaterLevel.toStringAsFixed(0)}%\n'
        'Stirring Speed: ${stirringSpeed.toStringAsFixed(0)} RPM\n'
        'Temp Thresholds: ${tempMin.toStringAsFixed(1)}°C - ${tempMax.toStringAsFixed(1)}°C\n'
        'pH Thresholds: ${phMin.toStringAsFixed(1)} - ${phMax.toStringAsFixed(1)} pH\n'
        'EC Thresholds: ${ecMin.toStringAsFixed(1)} - ${ecMax.toStringAsFixed(1)} ms/cm\n';
  }
}

// 2. Singleton Manager Class
// This instance will persist across screen changes
class ControlSettingsManager {
  static final ControlSettingsManager _instance =
      ControlSettingsManager._internal();

  factory ControlSettingsManager() {
    return _instance;
  }

  ControlSettingsManager._internal();

  // Internal state holding the latest settings (Initialized with default values)
  final Map<String, ControlSettings> _settingsMap = {
    'System 1': ControlSettings(
      targetWaterLevel: 70.0,
      stirringSpeed: 100.0,
      lightIntensity: 300,
      lightDuration: 12 * 3600,
      // Slow logging interval
      //primaryLogInterval: 7200.0, // Default 2 hours
      // samplingLogInterval: 43200.0, // Default 12 hours
      // Fast logging interval
      primaryLogInterval: 5,
      samplingLogInterval: 10,
      tempMin: 20.0,
      tempMax: 28.0,
      phMin: 6.5,
      phMax: 9.0,
      ecMin: 0.5,
      ecMax: 2.5,
      fcmToken: "initial_token_placeholder",
      harvestAutoEnabled: false,
      harvestIsRequested: false,
      harvestMinTime: 8,
      harvestRgbThreshold: 3.5,
      harvestTurbidityThreshold: 3.5,
      nutrientAutoEnabled: false,
      nutrientIsRequested: false,
      nutrientEcThreshold: 2.0,
      nutrientMinTime: 2.0,
    ),
    'System 2': ControlSettings(
      targetWaterLevel: 60.0,
      stirringSpeed: 50.0,
      lightIntensity: 0,
      lightDuration: 0 * 3600,
      // Slow logging interval
      //primaryLogInterval: 7200.0, // Default 2 hours
      //samplingLogInterval: 43200.0, // Default 12 hours
      // Fast logging interval
      primaryLogInterval: 5,
      samplingLogInterval: 10,
      tempMin: 20.0,
      tempMax: 28.0,
      phMin: 6.5,
      phMax: 8.0,
      ecMin: 0.5,
      ecMax: 2.5,
      fcmToken: "initial_token_placeholder",
      harvestAutoEnabled: false,
      harvestIsRequested: false,
      harvestMinTime: 8,
      harvestRgbThreshold: 3.5,
      harvestTurbidityThreshold: 3.5,
      nutrientAutoEnabled: false,
      nutrientIsRequested: false,
      nutrientEcThreshold: 2.0,
      nutrientMinTime: 2.0,
    ),
  };

  // Getter for the current settings
  ControlSettings getSettings(String systemId) {
    return _settingsMap[systemId] ?? _settingsMap['System 1']!;
  }

  // Setter method to update all settings for a specific system
  void updateSettings(String systemId, ControlSettings newSettings) {
    _settingsMap[systemId] = newSettings;
    debugPrint('Settings updated for $systemId:');
    debugPrint(_settingsMap[systemId].toString());
  }

  // Setter functions accept a systemId
  void _updateSpecificSetting(
      String systemId, ControlSettings Function(ControlSettings) copyFunction) {
    final current = getSettings(systemId);
    final updated = copyFunction(current);
    _settingsMap[systemId] = updated;
  }

  // Setter for individual slider values (used by the ControlScreen sliders)
  void setWaterLevel(String systemId, double level) {
    _updateSpecificSetting(
        systemId, (c) => c.copyWith(targetWaterLevel: level));
  }

  void setStirringSpeed(String systemId, double speed) {
    _updateSpecificSetting(systemId, (c) => c.copyWith(stirringSpeed: speed));
  }

  void setLightIntensity(String systemId, double lightIntensity) {
    _updateSpecificSetting(
        systemId, (c) => c.copyWith(lightIntensity: lightIntensity));
  }

  void setLightDuration(String systemId, double lightDuration) {
    _updateSpecificSetting(
        systemId, (c) => c.copyWith(lightDuration: lightDuration));
  }

  void setPrimaryLogInterval(String systemId, double interval) {
    _updateSpecificSetting(
        systemId, (c) => c.copyWith(primaryLogInterval: interval));
  }

  void setSamplingLogInterval(String systemId, double interval) {
    _updateSpecificSetting(
        systemId, (c) => c.copyWith(samplingLogInterval: interval));
  }
}
