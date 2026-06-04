# --- Network Configuration ---
BROKER_ADDRESS = "localhost"
BROKER_PORT = 1883

# --- SQLite Database Configuration ---
# CHANGE TO YOUR SQLITE DATABASE
DB_NAME = "/home/bee/Final_system/algae_project.db"  
BACKLOG_CHECK_INTERVAL_SEC = 120

# ---Firebase Configuration ---
# Path to Service Account JSON file (Used for authentication)
# CHANGE TO YOUR FIRESTORE DATABASE JSON FILE
# main database
FIREBASE_CREDENTIALS_PATH = "/home/bee/Final_system/smart-microalgae-cultivation-firebase-adminsdk-fbsvc-9bbb9d4d62.json"
# shared database
FIREBASE_SHARE_CREDENTIALS_PATH = "/home/bee/Final_system/microalgaeproject-45b1f-firebase-adminsdk-fbsvc-3204280aec.json"

# Firestore collection path where the data should be saved
MAIN_TANK_COLLECTION = "main_tank_data"
SAMPLING_TANK_COLLECTION = "sampling_tank_data"

# --- Client Settings ---
CLIENT_ID = "Pi_BiDir_Client"

# --- Camera ---
# CHANGE TO YOUR PI IMAGE FOLDER
SAVE_PATH = "/home/bee/Camera_GCS/images/"
# CHANGE TO YOUR GCS JSON FILE
CREDENTIALS_FILE = '/home/bee/Final_system/microalgae-pics-bf0d9aabd7b0.json'
# CHANGE TO YOUR GCS BUCKET NAME
BUCKET_NAME = 'rpi-gcs-pics'