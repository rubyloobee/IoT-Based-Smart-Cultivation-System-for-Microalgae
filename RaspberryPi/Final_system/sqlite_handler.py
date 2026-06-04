import sqlite3
from config import DB_NAME

def init_db():
    """Creates the tables if they don't exist."""
    try:
        # Conect with the database file, SQLite creates the file if it does not exist
        with sqlite3.connect(DB_NAME) as conn:
            # Cursor object as a pointer to execute SQL commands and retrieve results from database
            cursor = conn.cursor()
            
            # --- Main Tank Table ---
            # id INTEGER PRIMARY KEY AUTOINCREMENT: define a unique id for each row
            # TEXT: store the data as string
            # REAL: store the data as floating-point numbers
            # "uploaded": 0 - not uploaded/fail upload, 1 - uploaded
            # Pi periodically checks local database where uploaded = 0, push the backlog to Firestore (Store-and-Forward)
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS main_tank_logs (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    system_id TEXT,
                    timestamp TEXT, 
                    temperature_C REAL,
                    light_intensity_lux REAL,
                    water_level_pct REAL,
                    pH_value REAL,
                    up_private INTEGER DEFAULT 0,
                    up_share INTEGER DEFAULT 0
                )
            ''')

            # --- Sampling Tank Table ---
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS sampling_tank_logs (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    system_id TEXT,
                    timestamp TEXT,
                    EC_mscm REAL,
                    absorbance_RGB REAL,
                    absorbance_turbidity REAL,
                    up_private INTEGER DEFAULT 0,
                    up_share INTEGER DEFAULT 0
                )
            ''')
            
            # Finalise the table changes/creations and make them permanent in database file
            conn.commit()
            print(f"Database '{DB_NAME}' initialized successfully.")
            return True
           
    # Only handle errors that are related to SQLite database operations
    except sqlite3.Error as e:
        print(f"Database error during init: {e}")
        return False

def insert_main_data(system_id, data_dict):
    """Inserts data into main_tank_logs using specific JSON keys."""
    try:
        with sqlite3.connect(DB_NAME) as conn:
            cursor = conn.cursor()
            
            # Use data_dict.get() to safely retrieve values
            # If a key is missing in the JSON, it inserts None (NULL)
            # ? : parameter placeholders, indicate the actual values for insertion
            #     will be provided separately when SQL statement is executed
            cursor.execute('''
                INSERT INTO main_tank_logs (system_id, timestamp, temperature_C, light_intensity_lux, water_level_pct, pH_value)
                VALUES (?, ?, ?, ?, ?, ?)
            ''', (
                system_id,
                data_dict.get('timestamp'), 
                data_dict.get('temperature_C'), 
                data_dict.get('light_intensity_lux'), 
                data_dict.get('water_level_pct'),
                data_dict.get('pH_value')
            ))
            conn.commit()
            print(f">> Saved Main data in SQLite: {data_dict.get('timestamp')}")
            return True
        
    except sqlite3.Error as e:
        print(f"Error inserting main data: {e}")
        return False

def insert_sampling_data(system_id, data_dict):
    """Inserts data into sampling_tank_logs using specific JSON keys."""
    try:
        with sqlite3.connect(DB_NAME) as conn:
            cursor = conn.cursor()
            
            cursor.execute('''
                INSERT INTO sampling_tank_logs (system_id, timestamp, EC_mscm, absorbance_RGB, absorbance_turbidity)
                VALUES (?, ?, ?, ?, ?)
            ''', (
                system_id,
                data_dict.get('timestamp'), 
                data_dict.get('EC_mscm'),
                data_dict.get('absorbance_RGB'),
                data_dict.get('absorbance_turbidity')
            ))
            conn.commit()
            print(f">> Saved Sampling data in SQLite: {data_dict.get('timestamp')}")
            return True
        
    except sqlite3.Error as e:
        print(f"Error inserting sampling data: {e}")
        return False
    
def update_upload_flag(timestamp, db_type):
    """Updates the uploaded flag to 1 for a given record."""
    column = "up_private" if db_type == "private" else "up_share"
    try:
        with sqlite3.connect(DB_NAME) as conn:
            cursor = conn.cursor()
            
            # Since the timestamp should be unique across both logs, update both tables
            query_main = f"UPDATE main_tank_logs SET {column} = 1 WHERE timestamp = ?"
            query_sample = f"UPDATE sampling_tank_logs SET {column} = 1 WHERE timestamp = ?"
            
            cursor.execute(query_main, (timestamp,))
            main_affected = cursor.rowcount
            
            cursor.execute(query_sample, (timestamp,))
            sample_affected = cursor.rowcount
            
            conn.commit()
            
            if main_affected > 0 or sample_affected > 0:
                print(f"-> Local {column} flag set to 1 for timestamp: {timestamp}")

    except sqlite3.Error as e:
        print(f"Error updating {column} flag: {e}")
        
def fetch_unuploaded_data():
    """Fetches all records from both logs where the 'uploaded' flag is 0."""
    unuploaded_records = {
        'main_tank': [],
        'sampling_tank': []
    }
    
    try:
        with sqlite3.connect(DB_NAME) as conn:
            cursor = conn.cursor()
            
            # Fetch unuploaded main tank data
            cursor.execute("SELECT system_id, timestamp, temperature_C, light_intensity_lux, water_level_pct, pH_value FROM main_tank_logs WHERE up_private = 0 OR up_share = 0")
            # cursor.description contains column names with their desciption (display size, type code...)
            # Get column names to create dictionaries for easy use
            main_columns = [col[0] for col in cursor.description]
            # cursor.fetchall contains data values belonging to the column names
            for row in cursor.fetchall():
                # Convert the tuple row into a dictionary
                unuploaded_records['main_tank'].append(dict(zip(main_columns, row)))
            
            # Fetch unuploaded sampling tank data
            cursor.execute("SELECT system_id, timestamp, EC_mscm, absorbance_RGB, absorbance_turbidity FROM sampling_tank_logs WHERE up_private = 0 OR up_share = 0")
            sampling_columns = [col[0] for col in cursor.description]
            for row in cursor.fetchall():
                unuploaded_records['sampling_tank'].append(dict(zip(sampling_columns, row)))

    except sqlite3.Error as e:
        print(f"Error fetching unuploaded data: {e}")
        
    return unuploaded_records