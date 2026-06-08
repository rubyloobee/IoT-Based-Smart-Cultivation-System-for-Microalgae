import 'package:firebase_messaging/firebase_messaging.dart';
import 'package:cloud_firestore/cloud_firestore.dart';
import 'package:flutter_local_notifications/flutter_local_notifications.dart';
import 'ntp_sync.dart';

class NotificationService {
  final FirebaseMessaging _fcm = FirebaseMessaging.instance;
  final FirebaseFirestore _db = FirebaseFirestore.instance;

  // Instance for foreground alerts
  static final FlutterLocalNotificationsPlugin _localPlugin =
      FlutterLocalNotificationsPlugin();

  // --- 1. Initialization for Token & Permissions ---
  Future<void> initialize(String systemId) async {
    // Pop up on phone asking permission from user to send notifications
    NotificationSettings settings = await _fcm.requestPermission(
      alert: true,
      badge: true,
      sound: true,
    );

    // User clicks "Allow"
    if (settings.authorizationStatus == AuthorizationStatus.authorized) {
      String? token = await _fcm.getToken();
      if (token != null) {
        await _saveTokenToFirestore(systemId, token);
      }
    }
  }

  Future<void> _saveTokenToFirestore(String systemId, String token) async {
    final String docId = systemId.toLowerCase().replaceAll(' ', '_');

    await _db.collection('data_thresholds').doc(docId).set({
      'fcmToken': token,
    }, SetOptions(merge: true));

    print("FCM Token synced for $systemId");
  }

  // Showing the notification banner when app is open
  Future<void> initForegroundNotifications() async {
    // 1. Create the high importance channel for Android
    const AndroidNotificationChannel channel = AndroidNotificationChannel(
      'high_importance_channel', // Channel ID matched with Pi
      'Algae Alerts',
      importance: Importance.max, // Ensures it pops up and makes sound
    );

    await _localPlugin
        .resolvePlatformSpecificImplementation<
            AndroidFlutterLocalNotificationsPlugin>()
        ?.createNotificationChannel(channel);

    // 2. Basic initialization (No click handlers)
    const InitializationSettings initSettings = InitializationSettings(
      android: AndroidInitializationSettings('@mipmap/ic_launcher'),
    );
    await _localPlugin.initialize(initSettings);

    // 3. Listen for FCM messages and trigger the local banner
    FirebaseMessaging.onMessage.listen((RemoteMessage message) {
      if (message.notification != null) {
        final phoneTime = TimeSyncService.getCurrentTime();
        print('--- Notification Triggered ---');
        print('Flutter Phone Time:  $phoneTime');
        _showLocalNotification(message, channel);
      }
    });
  }

  // Takes the data and physically draws the notification banner on the user's screen
  void _showLocalNotification(
      RemoteMessage message, AndroidNotificationChannel channel) {
    _localPlugin.show(
      message.notification.hashCode, // Notification unique ID
      message.notification!.title, // Sent from Pi
      message.notification!.body, // Sent from Pi
      NotificationDetails(
        android: AndroidNotificationDetails(
          channel.id,
          channel.name,
          importance: channel.importance,
          icon: '@mipmap/ic_launcher',
        ),
      ),
    );
  }
}
