import 'package:flutter/material.dart';
import 'package:firebase_core/firebase_core.dart';
import 'firebase_options.dart';
import 'main_navigation.dart';
import 'notification_service.dart';
import 'ntp_sync.dart';

class LoadingScreen extends StatefulWidget {
  const LoadingScreen({super.key});

  @override
  // create and instance of corresponding class to manage changing of data
  State<LoadingScreen> createState() => _LoadingScreenState();
}

class _LoadingScreenState extends State<LoadingScreen> {
  @override
  void initState() {
    super.initState();
    // Start connecting to Firebase as soon as this screen appears
    _initialiseSystem();
  }

  Future<void> _initialiseSystem() async {
    try {
      // 1. Initialize Firebase ('Asynchronous Gap')
      await Firebase.initializeApp(
        options: DefaultFirebaseOptions.currentPlatform,
      );
      print("Firebase Connected");

      // 2. Initialize Notification Service (Foreground listening only)
      final notificationService = NotificationService();
      await notificationService.initForegroundNotifications();

      await notificationService.initialize("System 1");
      await notificationService.initialize("System 2");

      // 3. Add a tiny delay (1s) so the user can actually see the brand icon
      await Future.delayed(const Duration(seconds: 1));

      // 4. Move to the main app
      if (!mounted) return;
      Navigator.of(context).pushReplacement(
        MaterialPageRoute(builder: (context) => const MainNavigationScreen()),
      );
    } catch (e) {
      print("Initialization Error: $e");
    }
  }

  @override
  Widget build(BuildContext context) {
    return const Scaffold(
      backgroundColor: Colors.white,
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            Icon(Icons.waves, size: 80, color: Colors.teal),
            SizedBox(height: 20),
            Text(
              'Initialising System Data...',
              style: TextStyle(fontSize: 18, color: Colors.teal),
            ),
            SizedBox(height: 40),
            CircularProgressIndicator(color: Colors.teal),
          ],
        ),
      ),
    );
  }
}
