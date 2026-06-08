import 'package:ntp/ntp.dart';

class TimeSyncService {
  static int _offset = 0;

  // If network does not allow NTP (eg. eduroam), will be stuck at loading screen
  // Below code only works if connected to hotspot network
  // static Future<void> syncClock() async {
  //   final ntpTime = await NTP.now();
  //   final deviceTime = DateTime.now();

  //   _offset =
  //       ntpTime.millisecondsSinceEpoch - deviceTime.millisecondsSinceEpoch;

  //   print("NTP Sync Complete");
  //   print("Offset: $_offset ms");
  // }

  // static int getCurrentTime() {
  //   return DateTime.now().millisecondsSinceEpoch + _offset;
  // }

  static Future<void> syncClock() async {
    try {
      // 1. Try Google first (usually very fast)
      final ntpTime = await NTP
          .now(lookUpAddress: 'time.google.com')
          .timeout(const Duration(seconds: 2));
      _offset = ntpTime.millisecondsSinceEpoch -
          DateTime.now().millisecondsSinceEpoch;
    } catch (e) {
      print("NTP Primary failed, trying backup...");
      try {
        // 2. Try the default pool as a backup
        final ntpTime = await NTP.now().timeout(const Duration(seconds: 2));
        _offset = ntpTime.millisecondsSinceEpoch -
            DateTime.now().millisecondsSinceEpoch;
      } catch (e2) {
        // 3. Complete failure - just use device time
        _offset = 0;
        print("All NTP syncs failed. Using device time.");
      }
    }
  }

  static int getCurrentTime() {
    return DateTime.now().millisecondsSinceEpoch + _offset;
  }
}
