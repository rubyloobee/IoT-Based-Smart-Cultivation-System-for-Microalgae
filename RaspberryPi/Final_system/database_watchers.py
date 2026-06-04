import json
import time
import firebase_admin
from datetime import datetime
from google.cloud.firestore_v1.base_query import FieldFilter
from notification import monitor_data, start_notification_watchers

# --- Cold Start & Sync Logic -----
def sync_on_startup(db, client, client_metadata):
    """Fetches control settings on boot."""
    print("Performing Cold Start sync...")
    
    private_app = firebase_admin.get_app('private_data')
    
    # 1. Sync actuator control
    # Get all documents in the 'system_controls' collection
    control_docs = db.collection("system_controls").stream()
    for doc in control_docs:
        system_id = doc.id
        data = doc.to_dict() # Converts the NoSQL data to a Python dictionary
        
        # Fill 'memory' to prevent listener from double-sending
        client_metadata['last_known_control'][system_id] = data
        
        start_notification_watchers(db, system_id, private_app)
        
        # --- Fetch last sampling data ---
        sampling_ref = db.collection("sampling_tank_data").document(system_id).collection("logs")
        latest_sampling = sampling_ref.order_by("timestamp", direction="DESCENDING").limit(1).get()

        sampling_key = f"{system_id}_sampling"
        if sampling_key not in client_metadata['last_known_log']:
            client_metadata['last_known_log'][sampling_key] = {}

        if latest_sampling:
            sensor_data = latest_sampling[0].to_dict()
            # Merge EC value and timestamp into the cache
            client_metadata['last_known_log'][sampling_key].update(sensor_data)
        
        # --- Fetch last nutrient delivery timing ---
        activity_ref = db.collection("system_activity").document(system_id).collection("log")
        nutrient_query = (activity_ref
                          .where(filter=FieldFilter("type", "==", "Nutrient Delivery"))
                          .order_by("__name__", direction="DESCENDING")
                          .limit(1))
        latest_nutrient_activity = nutrient_query.get()
        
        last_dose_ts = 0
        if latest_nutrient_activity:
            try:
                # Using the document ID (timestamp string), convert to datetime object
                last_dose_ts = datetime.strptime(latest_nutrient_activity[0].id, "%Y-%m-%d %H:%M:%S").timestamp()
                print(f"[{system_id}] Confirmed last nutrient delivery: {latest_nutrient_activity[0].id}")
            except Exception as e:
                print(f"Error parsing nutrient activity for {system_id}: {e}")
        
        # Store in the sampling cache
        sampling_key = f"{system_id}_sampling"
        if sampling_key not in client_metadata['last_known_log']:
            client_metadata['last_known_log'][sampling_key] = {}
        client_metadata['last_known_log'][sampling_key]['last_nutrient_time'] = last_dose_ts
        
        # --- Fetch last harvest delivery timing ---
        harvest_query = (activity_ref
                          .where(filter=FieldFilter("type", "==", "Harvest"))
                          .order_by("__name__", direction="DESCENDING")
                          .limit(1))
        latest_harvest_activity = harvest_query.get()
        
        last_harvest_ts = 0
        if latest_harvest_activity:
            try:
                last_harvest_ts = datetime.strptime(latest_harvest_activity[0].id, "%Y-%m-%d %H:%M:%S").timestamp()
                print(f"[{system_id}] Confirmed last harvest: {latest_harvest_activity[0].id}")
            except Exception as e:
                print(f"Error parsing harvest history for {system_id}: {e}")
        
        # Store in the sampling cache
        sampling_key = f"{system_id}_sampling"
        if sampling_key not in client_metadata['last_known_log']:
            client_metadata['last_known_log'][sampling_key] = {}
        client_metadata['last_known_log'][sampling_key]['last_harvest_time'] = last_harvest_ts
        
        
        # Calculate initial light power
        duration_sec = data.get('target_light_duration', 0)
        dt = datetime.now()
        seconds_since_midnight = (dt.hour * 3600) + (dt.minute * 60) + dt.second
        # CHANGE TO SELECTED START TIME
        start_time_sec = 8 * 3600 # 8:00 AM
        
        should_be_on = start_time_sec <= seconds_since_midnight < (start_time_sec + duration_sec)
        light_power = "on" if should_be_on else "off"
    
        # Extract all control parameters 
        payload = {
            "type": "FULL_SYNC",  # Tells ESP32 to update all control parameters
            "light_power": light_power,
            "target_light_intensity": data.get('target_light_intensity'),
            "target_stirring_speed": data.get('target_stirring_speed'),
            "target_water_level": data.get('target_water_level')
        }
        
        # Publish to system-specific topic
        topic = f"{system_id}/control"
        client.publish(topic, json.dumps(payload), qos=1)
        
        print(f"Initialized & Synced {system_id} to {topic}")
        print(f"   Payload Sent: {payload}\n")
        
    # 2. Sync logging interval
    log_docs = db.collection("log_interval").stream()
    for doc in log_docs:
        system_id = doc.id
        data = doc.to_dict() # Converts the NoSQL data to a Python dictionary
        
        # Fill 'memory' to prevent listener from double-sending
        client_metadata['last_known_log'][system_id] = data
        
        print(f"Synced Logging Config for {system_id}:")
        print(f"   - Primary Interval : {data.get('primary_log_interval')}s")
        print(f"   - Sampling Interval: {data.get('sampling_log_interval')}s\n")

# --- Firestore Snapshot Listeners (Detecting Live Web Changes) ---
def start_database_watchers(db, client, client_metadata, scheduler):
    """Initializes the Firestore snapshot listeners."""   

    # Detect change in control settings
    def on_control_change(col_snapshot, changes, read_time):
        for change in changes:
            if change.type.name == 'MODIFIED':
                system_id = change.document.id
                new_data = change.document.to_dict()
                old_data = client_metadata['last_known_control'].get(system_id, {})
                
                client_metadata['last_known_control'][system_id] = new_data
                
                # Find only the modified field
                diff = {"type": "UPDATE"} # Tells ESP32 this is a single control parameter change
                fields = ['target_light_intensity','target_stirring_speed', 'target_water_level'] 
                
                for key in fields:
                    if new_data.get(key) != old_data.get(key):
                        diff[key] = new_data.get(key)
                        
                new_dur = new_data.get('target_light_duration')
                old_dur = old_data.get('target_light_duration')

                if new_dur != old_dur:
                    # This confirms the Pi sees the Firestore update
                    print(f"\n[Firestore] LIGHT DURATION CHANGE detected for {system_id}:")
                    print(f"   >> Old Duration: {old_dur}s")
                    print(f"   >> New Duration: {new_dur}s")
                    print(f"   >> Resetting scheduler memory to re-calculate state...")
                    
                    if scheduler and system_id in scheduler.light_state:
                        del scheduler.light_state[system_id]
                
                # Only publish if something changed
                if len(diff) > 1: 
                    
                    topic = f"{system_id}/control"
                    client.publish(topic, json.dumps(diff), qos=1)
                    print(f"[Control Update] sent for {system_id}: {diff}\n")
                
    # Detect change in logging settings
    def on_log_change(col_snapshot, changes, read_time):
        for change in changes:
            if change.type.name == 'MODIFIED':
                system_id = change.document.id
                new_data = change.document.to_dict()
                old_data = client_metadata['last_known_log'][system_id]
                
                new_p = round(float(new_data.get('primary_log_interval', 0)), 1)
                new_s = round(float(new_data.get('sampling_log_interval', 0)), 1)
                
                old_p = round(float(old_data.get('primary_log_interval', 0)), 1)
                old_s = round(float(old_data.get('sampling_log_interval', 0)), 1)
                
                if new_p != old_p or new_s != old_s:
                    # Update global memory
                    client_metadata['last_known_log'][system_id] = new_data
                    
                    print(f"[Logging Update] for {system_id}:")
                    print(f"   - Primary : {old_p}s -> {new_p}s")
                    print(f"   - Sampling: {old_s}s -> {new_s}s\n")
                    
                    # Ensure the Pi is running before reset the timer to count up to new interval
                    if 'scheduler' in globals():
                        scheduler.next_primary[system_id] = time.time() + new_p
                        scheduler.next_sampling[system_id] = time.time() + new_s
                
                
    # Set up listeners
    watch_control = db.collection("system_controls").on_snapshot(on_control_change)
    watch_log = db.collection("log_interval").on_snapshot(on_log_change)
    
    return watch_control, watch_log