import paho.mqtt.client as mqtt
from paho.mqtt.client import CallbackAPIVersion
import firebase_admin
from firebase_admin import credentials, firestore
import json, time

from subscriber import on_message, process_and_upload_backlog
from config import *
from scheduler import SystemScheduler
from sqlite_handler import init_db
from database_watchers import start_database_watchers, sync_on_startup
from camera import start_camera_watcher

# --- 1. Initialization ---
db_private = None
db_share = None
try:
    if 'private_data' not in firebase_admin._apps:
        # Load the credentials file
        cred_private = credentials.Certificate(FIREBASE_CREDENTIALS_PATH)
        firebase_admin.initialize_app(cred_private, name = 'private_data')
    db_private = firestore.client(app=firebase_admin.get_app('private_data'))
    print("Private Firebase Initialized.")
except Exception as e:
    print(f"Error Private Firebase: {e}")
    
try:
    if 'share_data' not in firebase_admin._apps:
        # Load the credentials file
        cred_share = credentials.Certificate(FIREBASE_SHARE_CREDENTIALS_PATH)
        firebase_admin.initialize_app(cred_share, name = 'share_data')
    db_share = firestore.client(app=firebase_admin.get_app('share_data'))
    print("Shared Firebase Initialized.")
except Exception as e:
    print(f"Error Shared Firebase: {e}")

# Initialise local database
init_db()

client_metadata = {
    'db_private': db_private,
    'db_share': db_share,
    'last_known_control': {},
    'last_known_log': {},
    'scheduler': None
}

# --- Connection Handlers ---
# called when the client connects to the broker
def on_connect(client, userdata, flags, reason_code, properties):
    """Handles both subscriptions (Code 1) and Cold Start Sync (Code 2)."""
    if reason_code == 0:
        print(f"Connected to MQTT Broker (Code: {reason_code})")
        
        # A. Subscribe to ESP32 Data Topics for any system
        subscription_list = [
            ("+/main_tank_data", 1), 
            ("+/sampling_tank_data", 1)
        ]
        client.subscribe(subscription_list)
        
        print(f"Subscribed to data topics: {subscription_list}")
        
        # B. Perform Cold Start Sync
        sync_on_startup(userdata['db_private'], client, userdata)
    else:
        print(f"Connection failed, reason code: {reason_code}")
    
# Error handling when client attempts to connect or reconnect to the MQTT broker
# used when using background network loop functions like client.loop_start() / client.loop_forever()
def on_connect_fail(client, userdata):
    print("Connection failed, Paho will retry...")


# --- Main Execution ---
# Initialize MQTT Client with Version 2 API and metadata dictionary
client = mqtt.Client(CallbackAPIVersion.VERSION2, CLIENT_ID, userdata=client_metadata)
# Assign handlers
client.on_connect = on_connect
client.on_connect_fail = on_connect_fail
client.on_message = on_message


client.connect(BROKER_ADDRESS, BROKER_PORT, keepalive=60)
client.loop_start()


# Initialise scheduler
scheduler = SystemScheduler(client, client_metadata['last_known_log'], client_metadata['last_known_control'], db_private = db_private)
client_metadata['scheduler'] = scheduler
# Start the background worker thread
scheduler.start()

# Database Watchers
watch_control, watch_log = start_database_watchers(db_private, client, client_metadata, scheduler)


# Start watcher for camera for system_1 only
camera_watch = start_camera_watcher(db_private, "system_1")

try:
    while True:
        db_p = client_metadata.get('db_private')
        db_s = client_metadata.get('db_share')
        
        if db_p:
            process_and_upload_backlog(db_p, db_s)
        
        # This MUST be outside any 'if' blocks to ensure the loop sleeps
        time.sleep(BACKLOG_CHECK_INTERVAL_SEC)
except KeyboardInterrupt:
    watch_control.unsubscribe()
    watch_log.unsubscribe()
    client.loop_stop()
    print("System Shutdown.")