import time
import json
import threading
from datetime import datetime
from notification import send_notification
import firebase_admin

class SystemScheduler:
    # Constructor for LoggingScheduler class
    def __init__(self, mqtt_client, shared_log, shared_control, db_private):
        # Store MQTT client created in pi_controller.py
        self.client = mqtt_client
        self.shared_log = shared_log
        self.shared_control = shared_control
        self.db_private = db_private
        # Store the 'next run time' for each system
        self.next_primary = {}  # { 'system_1': timestamp }
        self.next_sampling = {}
        self.light_state = {}
        self.last_seen = {} # Track last MQTT message time: { 'system_1_main': timestamp }
        self.timeout_sent = {} # Prevent notification spam: { 'system_1_main': True/False }
        self.running = True # flag check to esnure pi finishes its process before thread dies

    def start(self):
        """Starts the timer loop in a background thread."""
        # Execute _loop function, kill thread automatically if main program stops
        thread = threading.Thread(target=self._loop, daemon=True)
        thread.start()

    def _loop(self):
        while self.running:
            now = time.time()
            
            # All available keys (system_1, system_1_sampling, etc.)
            all_keys = list(self.shared_log.keys())
            
            # Filtered keys for logging (only system_1, system_2)
            base_systems = [s for s in all_keys if not s.endswith('_sampling')]
            
            # Handlet logging
            for system_id in base_systems:
                config = self.shared_log[system_id]
                self._handle_logging(system_id, config, now)

            # Handle MQTT timeout
            for system_id in all_keys:
                config = self.shared_log[system_id]
                self._check_mqtt_timeout(system_id, config, now)
                
            control_systems = list(self.shared_control.keys())
            for system_id in control_systems:
                control = self.shared_control[system_id]
                self._handle_photoperiod(system_id, control)
                self._handle_nutrients(system_id, control)
                self._handle_harvesting(system_id, control)
            
            time.sleep(1) # Check every second
            
    def _handle_logging(self, system_id, config, now):
        # 3600 and 43200 are fallback defaults
        p_interval = config.get('primary_log_interval', 3600)
        s_interval = config.get('sampling_log_interval', 43200)
        
        # Ensure last logging time exist, initialise if first run
        if not hasattr(self, 'last_primary_run'):
            self.last_primary_run = {}
        if not hasattr(self, 'last_sampling_run'):
            self.last_sampling_run = {}
            
        # --- Handle Primary Sensors ---
        last_p = self.last_primary_run.get(system_id)
        
        # If never run, or if the elapsed time exceeds the current interval
        if last_p is None or (now - last_p) >= p_interval:
            self._trigger_log(system_id, "get_main_data")
            self.last_primary_run[system_id] = now
            # Update next_primary
            self.next_primary[system_id] = now + p_interval
        
        # --- Handle Sampling Sensors ---
        last_s = self.last_sampling_run.get(system_id)
        
        if last_s is None or (now - last_s) >= s_interval:
            self._trigger_log(system_id, "get_sampling_data")
            self.last_sampling_run[system_id] = now
            # Update next_sampling for external tracking/UI
            self.next_sampling[system_id] = now + s_interval
    
                
    def _trigger_log(self, system_id, log_type):
        topic = f"{system_id}/log_sensor"
        payload = {"type": log_type}
        self.client.publish(topic, json.dumps(payload), qos=1)
        print(f"\nTimer reached for {system_id}: Sent {log_type} to {topic}")
                
    def _handle_photoperiod(self, system_id, control):
        duration_sec = control.get('target_light_duration', 0)
        
        # Get current time of day in seconds since midnight
        dt = datetime.now()
        seconds_since_midnight = (dt.hour * 3600) + (dt.minute * 60) + dt.second
        
        # Define start time (e.g., 12 PM = 43200 seconds)
        start_time_sec = 12 * 3600  

        end_time_sec = start_time_sec + (duration_sec)

        # Determine if light should be on
        # < for end time to prevent "double trigger" at the exact final second
        should_be_on = start_time_sec <= seconds_since_midnight < end_time_sec 
        
        new_state = "on" if should_be_on else "off"
        
        # Only send MQTT if the state has changed (to avoid flooding)
        # self.light_state is empty when code runs, hence always trigger when system start up
        if self.light_state.get(system_id) != new_state:
            self.light_state[system_id] = new_state
            self._trigger_light(system_id, new_state)
            
    def _trigger_light(self, system_id, state):
        topic = f"{system_id}/control"
        payload = {
            "type": "UPDATE",
            "light_power": state
        }
        self.client.publish(topic, json.dumps(payload), qos=1)
        print(f"[Photoperiod] {system_id} light turned {state.upper()}")
        
    def _handle_nutrients(self, system_id, control):
        """Checks EC thresholds and timers to trigger delivery."""
        
        now = time.time()
        
        # Get sensor data
        sampling_cache = self.shared_log.get(f"{system_id}_sampling", {})
        current_ec = float(sampling_cache.get('EC_mscm', 99.0))
    
        # 1. IMMEDIATE CHECK: Manual Request (Checks every 1s from local cache)
        # This is reactive to the database watcher
        if control.get('nutrient_is_requested', False):
            self._trigger_nutrient_delivery(system_id, "manual", current_ec)
            return

        # 2. AUTO CHECK: Triggered by new data in the subscriber cache
        # Look for the sampling data key updated by subscriber.py
        last_data_ts = sampling_cache.get('timestamp')
        
        # Only run auto-logic if this is a brand new sensor reading
        if last_data_ts and last_data_ts != getattr(self, f'_last_nutrient_processed_{system_id}', None):
            # Mark this timestamp as 'processed'
            setattr(self, f'_last_nutrient_processed_{system_id}', last_data_ts)
            
            # Extract values for the decision
            last_dose_time = sampling_cache.get('last_nutrient_time', 0)
            
            # User inputs
            auto_enabled = control.get('nutrient_auto_enabled', False)
            min_interval = control.get('nutrient_min_time', 0)
            ec_threshold = float(control.get('nutrient_ec_threshold', 0.0))

            # Decision Logic
            time_ok = (now - last_dose_time) >= min_interval
            nutrient_low = current_ec <= ec_threshold

            #if auto_enabled and time_ok and nutrient_low:
                #self._trigger_nutrient_delivery(system_id, "auto", current_ec)
                
    def _trigger_nutrient_delivery(self, system_id, mode, ec_value):
            """Executes the physical delivery command and logs the activity."""
            # Update last nutrient time
            sampling_key = f"{system_id}_sampling"
            self.shared_log[sampling_key]['last_nutrient_time'] = time.time()
            
            # Send MQTT command to ESP32
            topic = f"{system_id}/action"
            payload = {"type": "nutrient_start"}
            self.client.publish(topic, json.dumps(payload), qos=1)
            
            now_str = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            activity_entry = {
                "trigger": mode,
                "type": "Nutrient Delivery",
                "value_at_event": {"ec": ec_value}
            }
            
            try:
                # Log system activity to database with timestamp as ID
                self.db_private.collection('system_activity').document(system_id).collection('log').document(now_str).set(activity_entry)

                # Reset the manual button in Firestore
                # Set flag for nutrient pump malfunction detection          
                self.db_private.collection('system_controls').document(system_id).update({
                    'nutrient_is_requested': False,
                    'waiting_for_ec_rise': True
                })
                
                print(f"[{system_id}] nutrient_start sent via {mode} trigger at {now_str}")
            except Exception as e:
                print(f"Failed to log nutrient activity for {system_id}: {e}")
                
    def _handle_harvesting(self, system_id, control):
        """Checks RGB and turbidity thresholds and timers to harvesting."""
        now = time.time()
        
        # Get cache data
        sampling_cache = self.shared_log.get(f"{system_id}_sampling", {})
        current_RGB = float(sampling_cache.get('absorbance_RGB', 0))
        current_turbidity = float(sampling_cache.get('absorbance_turbidity', 0))

    
        # 1. IMMEDIATE CHECK: Manual Request (Checks every 1s from local cache)
        if control.get('harvest_is_requested', False):
            self._trigger_harvest(system_id, "manual", current_RGB, current_turbidity)
            return

        # 2. AUTO CHECK: Triggered by new data in the subscriber cache
        # Look for the sampling data key updated by subscriber.py
        last_data_ts = sampling_cache.get('timestamp')
        
        # Only run auto-logic if this is a brand new sensor reading
        if last_data_ts and last_data_ts != getattr(self, f'_last_harvest_processed_{system_id}', None):
            # Mark this timestamp as 'processed'
            setattr(self, f'_last_harvest_processed_{system_id}', last_data_ts)
            
            # Extract values for the decision
            last_harvest_time = float(sampling_cache.get('last_harvest_time', 0))
            
            # User inputs
            auto_enabled = control.get('harvest_auto_enabled', False)
            min_interval = control.get('harvest_min_time', 0)
            turbidity_threshold = float(control.get('harvest_turbidity_threshold', 0.0))
            RGB_threshold = float(control.get('harvest_rgb_threshold', 0.0))

            # Decision Logic
            time_ok = (now - last_harvest_time) >= min_interval
            harvest_condition = (current_turbidity >= turbidity_threshold) and (current_RGB >= RGB_threshold)

            if auto_enabled and harvest_condition:
                self._trigger_harvest(system_id, "auto", current_RGB, current_turbidity)
                
    def _trigger_harvest(self, system_id, mode, RGB, turbidity):
            """Executes the physical harvest command and logs the activity."""
            # Update last harvest time
            sampling_key = f"{system_id}_sampling"
            self.shared_log[sampling_key]['last_harvest_time'] = time.time()
            
            # Send MQTT command to ESP32
            topic = f"{system_id}/action"
            payload = {"type": "harvest_start"}
            self.client.publish(topic, json.dumps(payload), qos=1)
            
            now_str = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            activity_entry = {
                "trigger": mode,
                "type": "Harvest",
                "value_at_event": {"rgb": RGB, "turbidity": turbidity}
            }
            
            try:
                # Log system activity to database with timestamp as ID
                self.db_private.collection('system_activity').document(system_id).collection('log').document(now_str).set(activity_entry)

                # Reset the manual button in Firestore
                if mode == "manual":
                    self.db_private.collection('system_controls').document(system_id).update({'harvest_is_requested': False})
                
                print(f"[{system_id}] harvest_start sent via {mode} trigger at {now_str}")
            except Exception as e:
                print(f"Failed to log harvest activity for {system_id}: {e}")
                
    def _send_timeout_alert(self, system_id, minutes):
        """Fetches token and sends the notification."""     
        doc_target = system_id.replace('_sampling', '')
        doc_ref = self.db_private.collection('data_thresholds').document(doc_target).get()

        rules = doc_ref.to_dict()
        # rules is now guaranteed to be a dictionary, but we check the token anyway
        token = rules.get('fcmToken')
        
        formatted_system_id = system_id.replace('_', ' ').title()
        
        if token:
            send_notification(
                token,
                f"MQTT Timeout ({formatted_system_id})",
                f"No data received for {minutes} seconds. Check ESP32 power and WiFi.",
                firebase_admin.get_app('private_data')
            )
                
    def _check_mqtt_timeout(self, system_id, config, now):
        """Checks if MQTT data is overdue based on the logging interval."""
        # Determine which interval to use based on key suffix
        is_sampling = system_id.endswith('_sampling')
        interval_key = 'sampling_log_interval' if is_sampling else 'primary_log_interval'
        interval = config.get(interval_key, 3600 if not is_sampling else 43200)
        
        p_int = config.get('primary_log_interval')
        s_int = config.get('sampling_log_interval')

        # Get the last timestamp of data collected for this specific system/tank
        last_time = self.last_seen.get(system_id)
        
        # Only check for timeouts if there is at least one message since startup
        if last_time:
            silence_duration = now - last_time
            
            # Logic: Alert if silence > 1x interval (plus a 5s grace period for network lag)
            if silence_duration > (interval + 5):
                if not self.timeout_sent.get(system_id):
                    self._send_timeout_alert(system_id, int(silence_duration))
                    self.timeout_sent[system_id] = True # Set to false to prevent retriggering of notification
            else:
                # Reset the alert flag if data is flowing
                self.timeout_sent[system_id] = False
    
                
    def stop(self):
        self.running = False