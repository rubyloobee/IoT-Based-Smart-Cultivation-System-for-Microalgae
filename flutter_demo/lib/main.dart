// essential line for building a standard Flutter application
import 'package:flutter/material.dart';
import 'loading_screen.dart';

void main() async {
  // 1. Ensure Flutter is ready to call native code (needed for Firebase)
  WidgetsFlutterBinding.ensureInitialized();

  // 2. Sets the widget tree's root.
  runApp(const SimpleGreetingApp());
}

// 2. The main app container (StatelessWidget)
class SimpleGreetingApp extends StatelessWidget {
  const SimpleGreetingApp({
    super.key,
  });

  @override
  Widget build(BuildContext context) {
    // 3. MaterialApp: Provides Material Design styling and navigation services.
    return MaterialApp(
      title:
          'Microalgae System', // title which appears in the operating system's task switcher
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(
          seedColor: Colors.teal,
        ),
        useMaterial3: true,
        visualDensity: VisualDensity.adaptivePlatformDensity,
      ),
      home: const LoadingScreen(),
    );
  }
}
