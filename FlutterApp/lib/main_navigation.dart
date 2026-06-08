import 'package:flutter/material.dart';
import 'dashboard_screen.dart';
import 'charts_screen.dart';
import 'control_screen.dart';

// Main container for navigation - widget
class MainNavigationScreen extends StatefulWidget {
  const MainNavigationScreen({super.key});

  @override
  // Create and return a corresponding State object
  State<MainNavigationScreen> createState() => _MainNavigationScreenState();
}

// Define actual state object
class _MainNavigationScreenState extends State<MainNavigationScreen> {
  int _selectedIndex = 0;

  String _currentSystemId = 'System 1';

  final List<String> _systemIds = ['System 1', 'System 2'];

  List<Widget> get _widgetOptions => <Widget>[
        MonitoringScreen(
            key: ValueKey(_currentSystemId + 'monitor'),
            systemId: _currentSystemId), // Index 0
        ChartsScreen(
            key: ValueKey(_currentSystemId + 'charts'),
            systemId: _currentSystemId), // Index 1
        ControlScreen(
            key: ValueKey(_currentSystemId + 'control'),
            systemId: _currentSystemId), // Index 2
      ];

  // List of titles for the AppBar
  final List<String> _pageTitles = <String>[
    'Real-Time Cultivation Conditions',
    'Historical Data Charts',
    'System Parameter Controls'
  ];

  // Updates the selected index and closes the drawer
  void _onItemTapped(int index) {
    setState(() {
      _selectedIndex = index;
    });
    Navigator.pop(context);
  }

  // Select a new system ID
  void _selectSystem(String newSystemId) {
    if (_currentSystemId != newSystemId) {
      setState(() {
        _currentSystemId = newSystemId;
      });
    }
    Navigator.pop(context);
  }

  // Helper method to build consistent Drawer items
  Widget _buildDrawerItem({
    required IconData icon,
    required String title,
    required int index,
    required Color primaryColor,
  }) {
    final isSelected = _selectedIndex == index;

    return ListTile(
      leading: Icon(
        icon,
        color: isSelected ? primaryColor : Colors.grey.shade700,
      ),
      title: Text(
        title,
        style: TextStyle(
          fontWeight: isSelected ? FontWeight.bold : FontWeight.normal,
          color: isSelected ? primaryColor : Colors.black87,
        ),
      ),
      selected: isSelected,
      onTap: () => _onItemTapped(index),
    );
  }

  // Helper method to build the system selection tiles
  Widget _buildSystemItem({required String systemId}) {
    final isSelected = _currentSystemId == systemId;
    final primaryColor = Theme.of(context).colorScheme.primary;

    return ListTile(
      leading: Icon(Icons.hub_outlined,
          color: isSelected ? primaryColor : Colors.grey),
      title: Text(
        systemId,
        style: TextStyle(
          fontWeight: isSelected ? FontWeight.bold : FontWeight.normal,
          color: isSelected ? primaryColor : Colors.black87,
        ),
      ),
      selected: isSelected,
      onTap: () => _selectSystem(systemId),
    );
  }

  @override
  Widget build(BuildContext context) {
    final primaryColor = Theme.of(context).colorScheme.primary;

    return Scaffold(
      appBar: AppBar(
        title: Text('${_pageTitles[_selectedIndex]}'),
        backgroundColor: primaryColor,
        foregroundColor: Colors.white,
        elevation: 4,
      ),
      body: _widgetOptions.elementAt(_selectedIndex),
      drawer: Drawer(
        child: Column(
          children: <Widget>[
            DrawerHeader(
              decoration: BoxDecoration(color: primaryColor),
              child: const Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                mainAxisAlignment: MainAxisAlignment.end,
                children: [
                  CircleAvatar(
                    radius: 30,
                    backgroundColor: Colors.white,
                    child: Icon(Icons.water_drop, size: 30, color: Colors.teal),
                  ),
                  SizedBox(height: 8),
                  Text(
                    'Smart Microalgae System',
                    style: TextStyle(
                      color: Colors.white,
                      fontSize: 18,
                      fontWeight: FontWeight.bold,
                    ),
                  ),
                ],
              ),
            ),

            // System Selection Header
            Padding(
              padding: const EdgeInsets.fromLTRB(16, 16, 16, 8),
              child: Text('Select System:',
                  style: TextStyle(
                      fontWeight: FontWeight.bold,
                      color: Colors.teal.shade700)),
            ),

            // System Selection Tiles
            ..._systemIds.map((id) => _buildSystemItem(systemId: id)).toList(),

            const Divider(),

            // Navigation Items
            _buildDrawerItem(
              icon: Icons.monitor_heart_outlined,
              title: 'Live Dashboard',
              index: 0,
              primaryColor: primaryColor,
            ),
            _buildDrawerItem(
              icon: Icons.history_toggle_off_outlined,
              title: 'Historical Charts',
              index: 1,
              primaryColor: primaryColor,
            ),
            _buildDrawerItem(
              icon: Icons.settings,
              title: 'System Controls',
              index: 2,
              primaryColor: primaryColor,
            ),

            const Spacer(),

            const Padding(
              padding: EdgeInsets.only(bottom: 20),
              child: Text(
                'Created by Loo Bee',
                style: TextStyle(color: Colors.grey),
              ),
            ),
          ],
        ),
      ),
    );
  }
}
