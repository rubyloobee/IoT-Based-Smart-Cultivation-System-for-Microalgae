import firebase_admin
from firebase_admin import credentials, firestore, messaging
import time

# --- Firebase Setup ---
# CHANGE TO YOUR FIREBASE SERVICE ACCOUNT JSON KEY
cred = credentials.Certificate("/home/bee/Firebase/smart-microalgae-cultivation-firebase-adminsdk-fbsvc-9bbb9d4d62.json")
if not firebase_admin._apps:
    firebase_admin.initialize_app(cred)
db = firestore.client()

SYSTEM_ID = "system_1" # Match your doc ID


def send_notification(token, title, body):
    """Sends a high-priority push notification to the user's phone."""
    try:
        message = messaging.Message(
            # Define standard message content
            notification=messaging.Notification(title=title, body=body),
            android=messaging.AndroidConfig(
                # Tell Android OS to wake the phone up immediately
                priority='high',
                notification=messaging.AndroidNotification(
                    channel_id='high_importance_channel',  # Channel ID matched with Flutter
                ),
            ),
            token=token,
        )
        messaging.send(message)
        print(f"Alert Sent:\n{title}\n{body}\n")
    
    except Exception as e:
        print(f"Error sending notification: {e}")

def monitor_main_tank(doc_snapshot, changes, read_time):
    """Callback triggered whenever a new sensor reading is uploaded to main_tank_data collection."""
    for doc in doc_snapshot:
        data = doc.to_dict()
        
        # 1. Fetch current user-defined limits and the phone's token
        rules = db.collection('data_thresholds').document(SYSTEM_ID).get().to_dict()
        if not rules or not rules.get('fcmToken'): return
        
        token = rules.get('fcmToken')

        # 2. Temperature monitoring
        temp = data.get('temperature_C')
        if temp:
            current_temp = float(temp)     
            if current_temp > float(rules['temp_max']) or current_temp < float(rules['temp_min']):
                send_notification(rules['fcmToken'], "Temperature Alert!", f"System 1 is at {temp}\u00B0C")
           
        # 3. pH monitoring
        ph = data.get('ph_value')
        if ph:
            current_ph = float(ph)     
            if current_ph > float(rules['ph_max']) or current_ph < float(rules['ph_min']):
                send_notification(rules['fcmToken'], "pH Alert!", f"System 1 is at {ph} pH")
                
def monitor_sampling_tank(doc_snapshot, changes, read_time):
    """Callback triggered whenever a new sensor reading is uploaded to main_tank_data collection."""
    for doc in doc_snapshot:
        data = doc.to_dict()
        
        # 1. Fetch current user-defined limits and the phone's token
        rules = db.collection('data_thresholds').document(SYSTEM_ID).get().to_dict()
        if not rules or not rules.get('fcmToken'): return
        
        token = rules.get('fcmToken')

        # 2. EC monitoring
        ec = data.get('EC_mS/cm')
        if ec:
            current_ec = float(ec)     
            if current_ec > float(rules['ec_max']) or current_ec < float(rules['ec_min']):
                send_notification(rules['fcmToken'], "Conductivity Alert!", f"System 1 is at {ec} mS/cm")

# --- Start Listening ---
# Listens to the latest document in main_tank_data collection
# Use DESCENDING to put newest data at the top of the list
# Listener 1: Main Tank (Temp and pH)
query_main = (db.collection('main_tank_data')
                  .document(SYSTEM_ID)
                  .collection('logs')
                  .order_by('timestamp', direction=firestore.Query.DESCENDING)
                  .limit(1))
# Only executes when new document added to main_tank_data
query_main.on_snapshot(monitor_main_tank)

# Listener 2: Sampling Tank (EC)
query_sampling = (db.collection('sampling_tank_data')
                      .document(SYSTEM_ID)
                      .collection('logs')
                      .order_by('timestamp', direction=firestore.Query.DESCENDING)
                      .limit(1))
query_sampling.on_snapshot(monitor_sampling_tank)

print("Alert Watchdog is active and listening...")
while True:
    time.sleep(1)  # Keep the main thread alive