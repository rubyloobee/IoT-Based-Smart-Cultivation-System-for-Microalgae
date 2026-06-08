import 'package:flutter/material.dart';
import 'package:fl_chart/fl_chart.dart';
import 'firestore_service.dart';
import 'sensor_model.dart';

class ChartsScreen extends StatefulWidget {
  final String systemId;
  const ChartsScreen({required this.systemId, super.key});

  @override
  State<ChartsScreen> createState() => _ChartsScreenState();
}

class _ChartsScreenState extends State<ChartsScreen> {
  final FirestoreService _firestoreService = FirestoreService();

  Widget _buildChartCard({
    required String title,
    required IconData icon,
    required Color color,
    required Stream<List<SensorData>> stream,
    required double Function(SensorData) valueSelector,
    required String unit,
    double? minY,
    double? maxY,
  }) {
    return Card(
      elevation: 6,
      margin: const EdgeInsets.only(bottom: 16),
      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(15)),
      child: Padding(
        padding: const EdgeInsets.fromLTRB(16, 12, 16, 8),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Row(
              children: [
                CircleAvatar(
                  backgroundColor: color,
                  radius: 12,
                  child: Icon(icon, color: Colors.white, size: 16),
                ),
                const SizedBox(width: 8),
                Text(
                  '$title Trend',
                  style: TextStyle(
                    fontSize: 18,
                    fontWeight: FontWeight.bold,
                    color: Colors.teal.shade700,
                  ),
                ),
              ],
            ),
            const SizedBox(height: 12),
            SizedBox(
              height: 280,
              child: StreamBuilder<List<SensorData>>(
                stream: stream,
                builder: (context, snapshot) {
                  if (!snapshot.hasData)
                    return const Center(child: CircularProgressIndicator());
                  final data = snapshot.data!;
                  if (data.isEmpty)
                    return const Center(child: Text("No Data Available"));

                  return LineChart(
                    LineChartData(
                      clipData: const FlClipData.all(),
                      minY: minY,
                      maxY: maxY,
                      gridData: FlGridData(
                        show: true,
                        drawVerticalLine: false,
                        horizontalInterval:
                            maxY != null ? (maxY - (minY ?? 0)) / 5 : null,
                      ),
                      titlesData: FlTitlesData(
                        leftTitles: AxisTitles(
                          axisNameWidget: Padding(
                            padding: const EdgeInsets.only(bottom: 4),
                            child: Text(
                              unit,
                              style: TextStyle(
                                  color: color,
                                  fontWeight: FontWeight.bold,
                                  fontSize: 16),
                            ),
                          ),
                          axisNameSize: 22,
                          sideTitles: SideTitles(
                            showTitles: true,
                            reservedSize: 45,
                            getTitlesWidget: (value, meta) {
                              return SideTitleWidget(
                                meta: meta,
                                child: Text(
                                  value.toStringAsFixed(1),
                                  style: const TextStyle(
                                      fontSize: 12,
                                      color: Colors.black87,
                                      fontWeight: FontWeight.w500),
                                ),
                              );
                            },
                          ),
                        ),
                        bottomTitles: AxisTitles(
                          sideTitles: SideTitles(
                            showTitles: true,
                            reservedSize: 85,
                            getTitlesWidget: (value, meta) {
                              int index = value.toInt();
                              if (index >= 0 && index < data.length) {
                                String ts = data[index].timestamp;
                                if (ts.length > 11) ts = ts.substring(5, 16);
                                return SideTitleWidget(
                                  meta: meta,
                                  space: 8,
                                  child: RotatedBox(
                                    quarterTurns: 1,
                                    child: Text(
                                      ts,
                                      style: const TextStyle(
                                          fontSize: 11, color: Colors.black54),
                                    ),
                                  ),
                                );
                              }
                              return const SizedBox();
                            },
                          ),
                        ),
                        rightTitles: const AxisTitles(
                            sideTitles: SideTitles(showTitles: false)),
                        topTitles: const AxisTitles(
                            sideTitles: SideTitles(showTitles: false)),
                      ),
                      borderData: FlBorderData(
                        show: true,
                        border: Border(
                          bottom: BorderSide(color: Colors.grey.shade300),
                          left: BorderSide(color: Colors.grey.shade300),
                        ),
                      ),
                      lineBarsData: [
                        LineChartBarData(
                          spots: data
                              .asMap()
                              .entries
                              .map((e) => FlSpot(
                                  e.key.toDouble(), valueSelector(e.value)))
                              .toList(),
                          isCurved: true,
                          color: color,
                          barWidth: 3,
                          dotData: const FlDotData(show: true),
                          belowBarData: BarAreaData(
                            show: true,
                            color: color.withOpacity(0.1),
                          ),
                        ),
                      ],
                    ),
                  );
                },
              ),
            ),
          ],
        ),
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    final mainStream =
        _firestoreService.getHistoricalSensorData(widget.systemId, 'main');
    final samplingStream =
        _firestoreService.getHistoricalSensorData(widget.systemId, 'sampling');

    return Container(
      color: Colors.teal.shade50,
      child: SingleChildScrollView(
        padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 16),
        child: Column(
          children: [
            _buildHeader(),

            // --- MAIN TANK SENSORS ---
            _buildChartCard(
                title: 'Temperature',
                icon: Icons.thermostat,
                color: Colors.red.shade400,
                stream: mainStream,
                valueSelector: (s) => s.temperature,
                unit: '°C',
                minY: 20,
                maxY: 35),
            _buildChartCard(
                title: 'pH Level',
                icon: Icons.science,
                color: Colors.green.shade400,
                stream: mainStream,
                valueSelector: (s) => s.pH,
                unit: 'pH',
                minY: 0,
                maxY: 14),
            _buildChartCard(
                title: 'Water Level',
                icon: Icons.water_drop,
                color: Colors.blue.shade400,
                stream: mainStream,
                valueSelector: (s) => s.waterLevel,
                unit: '%',
                minY: 0,
                maxY: 100),
            _buildChartCard(
                title: 'Light Intensity',
                icon: Icons.lightbulb,
                color: Colors.amber.shade600,
                stream: mainStream,
                valueSelector: (s) => s.lightIntensity,
                unit: 'Lux',
                minY: 0,
                maxY: 500),

            const SizedBox(height: 16),
            const Divider(thickness: 1.5),
            const SizedBox(height: 16),

            // --- SAMPLING TANK SENSORS ---
            _buildChartCard(
                title: 'Conductivity',
                icon: Icons.bolt,
                color: Colors.purple.shade400,
                stream: samplingStream,
                valueSelector: (s) => s.conductivity,
                unit: 'mS/cm',
                minY: 0,
                maxY: 3),
            _buildChartCard(
                title: 'Turbidity',
                icon: Icons.opacity,
                color: Colors.brown.shade400,
                stream: samplingStream,
                valueSelector: (s) => s.turbidity,
                unit: 'abs',
                minY: 0,
                maxY: 5),
            _buildChartCard(
                title: 'Colour Density',
                icon: Icons.color_lens,
                color: Colors.lime.shade700,
                stream: samplingStream,
                valueSelector: (s) => s.color,
                unit: 'abs',
                minY: 0,
                maxY: 5),

            const SizedBox(height: 20),
          ],
        ),
      ),
    );
  }

  Widget _buildHeader() {
    return Padding(
      padding: const EdgeInsets.only(bottom: 16),
      child: Row(
        children: [
          Container(
            height: 24,
            width: 4,
            decoration: BoxDecoration(
                color: Colors.teal.shade700,
                borderRadius: BorderRadius.circular(2)),
          ),
          const SizedBox(width: 12),
          Text(
            'Historical Trends',
            style: TextStyle(
                fontSize: 22,
                fontWeight: FontWeight.bold,
                color: Colors.grey.shade800),
          ),
        ],
      ),
    );
  }
}
