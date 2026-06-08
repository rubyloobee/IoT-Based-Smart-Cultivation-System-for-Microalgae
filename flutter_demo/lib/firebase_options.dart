import 'package:firebase_core/firebase_core.dart';
import 'package:flutter/foundation.dart'
    show defaultTargetPlatform, TargetPlatform;

class DefaultFirebaseOptions {
  static FirebaseOptions get currentPlatform {
    // This logic ensures the app uses the correct credentials for Android
    if (defaultTargetPlatform == TargetPlatform.android) {
      return android;
    }
    throw UnsupportedError(
      'DefaultFirebaseOptions are not configured for this platform.',
    );
  }

  // Change below to your Firebase

  static const FirebaseOptions android = FirebaseOptions(
    apiKey: 'AIzaSyC05M4wkyfgaTNVIe951ZaZiDc-2znlkGc',
    appId: '1:1037877776343:android:dd06757433fdd544504592',
    messagingSenderId: '1037877776343',
    projectId: 'smart-microalgae-cultivation',
    storageBucket: 'smart-microalgae-cultivation.firebasestorage.app',
  );
}
