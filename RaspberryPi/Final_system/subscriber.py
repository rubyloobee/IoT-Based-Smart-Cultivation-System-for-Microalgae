import paho.mqtt.client as mqtt
import time
from config import (
    MAIN_TANK_COLLECTION, SAMPLING_TANK_COLLECTION
)
import json
from sqlite_handler import insert_main_data, insert_sampling_data, update_upload_flag, fetch_unuploaded_data

def upload_to_firestore(db, collection_name, system_id, data, db_label):
    """Upload record to Firestore."""
    try:
        doc_id = data.get('timestamp') 

        # Creates copy of original sensor dictionary to prevent unintentionally altering the original dictionary
        upload_data = data.copy()

        # Upload using the timestamp as the document ID
        db.collection(collection_name).document(system_id).collection('logs').document(doc_id).set(upload_data)
        print(f"-> Successfully uploaded record {doc_id} {db_label} to Firestore : {collection_name}")
        return True

    except Exception as e:
        print(f"Error uploading to Firestore collection {collection_name}: {e}")
        return False

# Called when a message is received from the broker
# msg is the object containing the message details
def on_message(client, userdata, msg):
    """Processes incoming MQTT messages."""
    
    # Retrieve the initialised Firestore client object
    db_p = userdata.get('db_private')
    db_s = userdata.get('db_share')
    
    # MQTT Payload (message content) is bytes, decode to a string for printing
    payload_str = msg.payload.decode()
    parts = msg.topic.split('/')
    system_id = parts[0]
    topic_type = parts[1] if len(parts) > 1 else ""
    
    print(f"\n>>> Message received from ESP32 >>>")
    # topic attribute within msg.topic is the default variable name used by
    # Paho-MQTT library to store topic string associated with the received message
    print(f"Topic: {msg.topic}")               
    print(f"Payload: {payload_str}")
    
    # Parse the JSON string into a Python Dictionary
    # Allow access of any specific sensor reading using its key (sensor name)
    data = json.loads(payload_str)
    
    local_success = False
    collection = None
        
    # --- ACTIONS BASED ON TOPIC ---
    if "main_tank_data" in topic_type:
        print(f"\n>>> Main Data Received from {system_id} >>>")
        local_success = insert_main_data(system_id, data)
        collection = MAIN_TANK_COLLECTION
        
    elif "sampling_tank_data" in topic_type:
        print(f"\n>>> Sampling Data Received from {system_id} >>>")
        local_success = insert_sampling_data(system_id, data)
        collection = SAMPLING_TANK_COLLECTION
        # Update latest sampling data
        userdata['last_known_log'][f"{system_id}_sampling"].update(data)
        
    # --- IMMEDIATE CLOUD UPLOAD ATTEMPT  ---
    # if initialisation of db is successful, and local database upload is successful
    if local_success:
        if db_p and upload_to_firestore(db_p, collection, system_id, data, "private"):
            update_upload_flag(data.get('timestamp'), "private")
            
        if db_s and upload_to_firestore(db_s, collection, system_id, data, "shared"):
            update_upload_flag(data.get('timestamp'), "share")
    
    scheduler = userdata.get('scheduler')
    if scheduler:
        # Determine the key used in the scheduler's last_seen dictionary
        # If it's sampling data, key is "system_1_sampling", else "system_1"
        tank_suffix = "_sampling" if "sampling_tank_data" in msg.topic else ""
        cache_key = f"{system_id}{tank_suffix}"
        
        scheduler.last_seen[cache_key] = time.time()
            
def process_and_upload_backlog(db_private, db_share):
    """
    Checks the local database for unuploaded records and attempts to upload them.
    Implements the 'Store-and-Forward' pattern.
    """
    """Modified to sync backlog to both databases."""
        
    backlog = fetch_unuploaded_data()
    p_count, s_count = 0, 0
    
    categories = [
        ('main_tank', MAIN_TANK_COLLECTION),
        ('sampling_tank', SAMPLING_TANK_COLLECTION)
    ]
    
    for key, coll_name in categories:
        for record in backlog[key]:
            sys_id = record.get('system_id')
            ts = record.get('timestamp')
            
            # Only upload to private if it hasn't been done yet
            if record.get('up_private') == 0 and db_private:
                if upload_to_firestore(db_private, coll_name, sys_id, record, "private"):
                    update_upload_flag(ts, "private")
                    p_count += 1

            # Only upload to share if it hasn't been done yet
            if record.get('up_share') == 0 and db_share:
                if upload_to_firestore(db_share, coll_name, sys_id, record, "shared"):
                    update_upload_flag(ts, "share")
                    s_count += 1
                    
        if p_count > 0 or s_count > 0:
            print(f"--- Backlog Sync Complete: Private({p_count}), Shared({s_count}) ---")
        else:
            print("--- All databases are up to date. ---")
            