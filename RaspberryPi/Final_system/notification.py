import firebase_admin
from firebase_admin import messaging, firestore
from datetime import datetime, timedelta
import time
from google.cloud.firestore_v1.base_query import FieldFilter


stuck_cache = {}
# Record time when script started 
script_startup_time = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

# Map database keys to professional display names
SENSOR_DISPLAY_NAMES = {
    'absorbance_turbidity': 'Turbidity',
    'absorbance_RGB': 'Colour Density',
    'EC_mscm': 'Conductivity',
    'pH_value': 'pH',
    'temperature_C': 'Temperature',
    'water_level_pct': 'Water Level',
    'light_intensity': 'Light Intensity'
}

# CHANGE TO YOUR TANK HEIGHT IN CM
TANK_MAX_HEIGHT = 24.0

def get_display_name(key):
    return SENSOR_DISPLAY_NAMES.get(key, key.replace('_', ' ').title())

def send_notification(token, title, body, app):
    """Sends a high-priority push notification."""
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
        messaging.send(message, app=app)
        print(f"Alert Sent:\n{title}\n{body}\n")
    
    except Exception as e:
        print(f"Error sending notification: {e}")
        


def check_hardware(data, token, formatted_system_id, app):
    failed_sensors = set()
    for key, value in data.items():
        if key == 'timestamp':
            continue
                
        # Check for specific hardware error code
        if float(value) < 0:
            send_notification(token, f"Hardware Failure ({formatted_system_id})", f"{get_display_name(key)} sensor reported a hardware error.", app)
            failed_sensors.add(key)
            continue
                
        # Check for physical Out-of-Bounds (Impossible values)
        is_oob = False
        if key == 'temperature_C' and (float(value) < -5 or float(value) > 70):
            send_notification(token, f"Sensor OOB ({formatted_system_id})", f"Impossible Temp detected: {value}", app)
            is_oob = True
                    
        elif key == 'pH_value' and (float(value) < 0 or float(value) > 14):
            send_notification(token, f"Sensor OOB ({formatted_system_id})", f"Impossible pH detected: {value}", app)
            is_oob = True
                    
        if is_oob:
            failed_sensors.add(key)
            
    return failed_sensors

def check_stuck_sensors(data, old_data, old_entry, tank_type, token, formatted_system_id, app, failed_sensors):
    for key, current_val in data.items():
        if key == 'timestamp' or key not in old_data:
            continue
                        
        if current_val == old_data[key]:
            # Increment counter
            count = old_entry['counters'].get(key, 0) + 1
            old_entry['counters'][key] = count
            
            # Main tank allow 1 repeat, alert on 2nd repeat (higher logging frequency)
            # Sampling tank alert on 1st repeat (lower logging frequency)
            threshold = 2 if tank_type == 'main' else 1
            
            if count >= threshold:
                send_notification(token, f"Stuck Sensor ({formatted_system_id})", 
                                    f"{get_display_name(key)} reading is stagnant for {count+1} readings.", app)
                failed_sensors.add(key)
                            
        else:
            # Reset counter if value moves
            old_entry['counters'][key] = 0
            
def check_actuator_malfunctions(data, old_data, tank_type, controls, token, formatted_system_id, app, db, system_id, failed_sensors):            
    nutrient_fail = False
    
    # 1. Nutrient Pump Check (Sampling Tank)
    if tank_type == 'sampling' and 'EC_mscm' in data and 'EC_mscm' not in failed_sensors:
        current_ec = float(data['EC_mscm'])
        previous_ec = float(old_data.get('EC_mscm'))
                        
        # Logic: Only check if the Pi recently triggered a delivery
        if controls.get('waiting_for_ec_rise') is True:
            # Prevent crash if EC sensor reads 0
            if previous_ec > 0:
                ec_rise_pct = ((current_ec - previous_ec) / previous_ec) * 100
            else:
                ec_rise_pct = 0
                        
            # Use a 2.5% threshold to clear 1.32% max noise floor of EC sensor
            if ec_rise_pct < 2.5:
                send_notification(token, f"Dosing Failure ({formatted_system_id})", 
                                                "Nutrient pump was triggered, but no increase in Conductivity (EC) was detected.", app)
                nutrient_fail = True # Suppresses stagnant growth alert 
            # Reset the flag so we don't alert again until the next trigger
            db.collection('system_controls').document(system_id).update({'waiting_for_ec_rise': False})
                     
    # 2. Water Level Pump Check (Main Tank)
    if tank_type == 'main' and 'water_level_pct' in data and 'water_level_pct' not in failed_sensors:
        current_LevelPct = float(data['water_level_pct'])
        previous_LevelPct = float(old_data.get('water_level_pct'))
                    
        # target water level from firestore
        target_LevelPct = float(controls.get('target_water_level', 0))
    
        # Calculate gap in percentage
        gap = target_LevelPct - current_LevelPct
                    
        # 1. Pump Failure Detection (Water level below target and not rising)
        if gap > 5:
            if current_LevelPct <= previous_LevelPct:
                send_notification(token, f"Pump Failure ({formatted_system_id})", 
                                    f"Level is {current_LevelPct}% but not rising. ESP32 should be pumping to reach {target_LevelPct}%.", app)                            
                
        # 2. Inverse Logic / Overfill Detection (At target but still rising)
        elif current_LevelPct >= target_LevelPct:
            if (current_LevelPct - previous_LevelPct) > 2:
                send_notification(token, f"Overfill Anomaly ({formatted_system_id})", 
                                    f"Tank is overfilled ({current_LevelPct}%). Rise detected during IDLE state. Possible Siphon or Stuck Relay.", app)    
    return nutrient_fail

def check_environmental_thresholds(data, tank_type, rules, failed_sensors, token, formatted_system_id, app):
    if tank_type == 'main':
        # Temperature monitoring
        temp = data.get('temperature_C')
        if temp and 'temperature_C' not in failed_sensors:
            if float(temp) > float(rules['temp_max']) or float(temp) < float(rules['temp_min']):
                send_notification(token, f"Temperature Alert ({formatted_system_id})", f"Value: {temp}\u00B0C", app)
                           
        # pH monitoring
        ph = data.get('pH_value')
        if ph and 'pH_value' not in failed_sensors:
            if float(ph) > float(rules['ph_max']) or float(ph) < float(rules['ph_min']):
                send_notification(token, f"pH Alert ({formatted_system_id})", f"Value: {ph} pH", app)

    elif tank_type == 'sampling':
        # EC monitoring
        ec = data.get('EC_mscm')
        if ec and 'EC_mscm' not in failed_sensors:
            if float(ec) > float(rules['ec_max']) or float(ec) < float(rules['ec_min']):
                send_notification(token, f"Conductivity Alert ({formatted_system_id})", f"Value: {ec} mS/cm", app)
 
def check_growth_disruption(data, old_data, db, system_id, nutrient_fail, failed_sensors, token, formatted_system_id, app):
    active_bio = [s for s in ['absorbance_turbidity', 'absorbance_RGB'] if s in data and s not in failed_sensors]
                
    if active_bio:
        # A. Sudden Growth Crash (between every sampling data)
        for sensor in active_bio:
            current_absorbance, previous_absorbance = float(data[sensor]), float(old_data.get(sensor))
            if current_absorbance > 0:
                absorbance_drop_pct = ((previous_absorbance - current_absorbance) / current_absorbance * 100)
                if absorbance_drop_pct > 15:
                    send_notification(
                        token,
                        f"Growth Crash ({formatted_system_id})",
                        f"{get_display_name(sensor)} absorbance dropped by {absorbance_drop_pct:.1f}%. Immediate inspection required.",
                        app
                    )

        # B. Stagnant Growth (24-Hour Check) - Suppressed if nutrient delivery failed
        if not nutrient_fail:
            # Get latest data which is at least 24 hours old
            one_day_ago = datetime.now() - timedelta(days=1)
            hist_absorbance = db.collection('sampling_tank_data').document(system_id).collection('logs')\
                                .where(filter=FieldFilter('timestamp', '>=', one_day_ago.isoformat()))\
                                .order_by('timestamp', direction='ASCENDING').limit(1).get()
                        
            # 24 hours old data exist
            if hist_absorbance:
                hist_data = hist_absorbance[0].to_dict()
                stagnant_votes = 0
                absorbance_stats = []
                for sensor in active_bio:
                    # Default to current value if historical log is missing
                    hist_val = float(hist_data.get(sensor, data[sensor]))
                                
                    if hist_val > 0:
                        # Each biomass sensors which shows growth of lesser than 3% cast a vote
                        absorbance_change_pct = (float(data[sensor]) - hist_val) / hist_val * 100
                        absorbance_stats.append(f"{get_display_name(sensor)}: {absorbance_change_pct:+.1f}%")
                                
                        if abs(absorbance_change_pct) < 3.0:
                            stagnant_votes += 1
                            
                # Only trigger if both healthy biomass sensors agree that nothing is happening
                if stagnant_votes == len(active_bio):
                    stats_str = ", ".join(absorbance_stats)
                    send_notification(
                        token, 
                        f"Stagnant Growth Detected ({formatted_system_id})", 
                        f"Growth stalled over 24h. Changes: {stats_str}. (Threshold < 3%)", 
                        app
                    )
                                
def monitor_data(doc_snapshot, changes, read_time, db, system_id, tank_type, app):
    """Callback triggered whenever a new sensor reading is uploaded."""
    # Loop every individual update
    for change in changes:
        # Only alert on new data entries
        if change.type.name in ['ADDED', 'MODIFIED']:
            # Get data timestamp
            doc_id = change.document.id
            data = change.document.to_dict()
            
            # Surpress notifications triggered by existing sensor logs
            if doc_id <= script_startup_time:
                continue
        
            # Fetch current user-defined limits and the phone's token
            rules = db.collection('data_thresholds').document(system_id).get().to_dict()
            if not rules or not rules.get('fcmToken'): return
            token = rules.get('fcmToken')

            if not token: return
            
            # Converts 'system_1' to 'System 1'
            formatted_system_id = system_id.replace('_', ' ').title()
            
            # --- SENSOR ERRORS & OUT-OF-BOUNDS (OOB) ---
            failed_sensors = check_hardware(data, token, formatted_system_id, app)
            
                
            # --- SETUP CACHE ---
            cache_key = f"{system_id}_{tank_type}"
            is_first_run = False
            
            if cache_key not in stuck_cache:
                stuck_cache[cache_key] = {'data': data, 'counters': {}}
                is_first_run = True
            
            old_entry = stuck_cache[cache_key]
            old_data = old_entry['data']
            
            #--- VALUE STUCK DETECTION ---
            if not is_first_run:
                check_stuck_sensors(data, old_data, old_entry, tank_type, token, formatted_system_id, app, failed_sensors)

            # Fetch control states to see if we were expecting changes
            controls = db.collection('system_controls').document(system_id).get().to_dict() or {}   
                
                
            # --- ACTUATOR MALFUNCTION DETECTION ---
            if not is_first_run:
                nutrient_fail = check_actuator_malfunctions(data, old_data, tank_type, controls, token, formatted_system_id, app, db, system_id, failed_sensors)
                
                
            # --- THRESHOLD MONITORING --
            check_environmental_thresholds(data, tank_type, rules, failed_sensors, token, formatted_system_id, app)
            
            # --- GROWTH DISRUPTION ---
            if tank_type == 'sampling' and not is_first_run:
                check_growth_disruption(data, old_data, db, system_id, nutrient_fail, failed_sensors, token, formatted_system_id, app)
                
            # Update cache with current data for the next run
            stuck_cache[cache_key]['data'] = data
            
def start_notification_watchers(db, system_id, app):
    """Sets up listeners for a specific system's sensor logs."""
    
    # Monitor main tank
    # Use DESCENDING to put newest data at the top of the list
    main_query = (db.collection('main_tank_data')
                  .document(system_id)
                  .collection('logs')
                  .order_by('timestamp', direction=firestore.Query.DESCENDING)
                  .limit(1))
    
    # We use a lambda to pass extra arguments (db, system_id) to the callback
    main_query.on_snapshot(lambda docs, changes, read_time: 
                           monitor_data(docs, changes, read_time, db, system_id, 'main', app))

    # Monitor sampling tank
    sampling_query = (db.collection('sampling_tank_data')
                      .document(system_id)
                      .collection('logs')
                      .order_by('timestamp', direction=firestore.Query.DESCENDING)
                      .limit(1))
    
    sampling_query.on_snapshot(lambda docs, changes, read_time: 
                               monitor_data(docs, changes, read_time, db, system_id, 'sampling', app))
    
    print(f"Notification Watchdog active for {system_id}")