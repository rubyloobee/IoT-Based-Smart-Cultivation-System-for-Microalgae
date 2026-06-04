# --- Camera & File System Configuration ---
# CHANGE TO THE PATH WHERE IMAGES ARE SAVED ON THE RASBPERRY PI
SAVE_PATH = "/home/bee/Camera_GCS/images/"

# 2. Capture interval in seconds
INTERVAL_SECONDS = 60

# --- Google Cloud Storage Configuration ---
# CHANGE TO YOUR GCS SERVICE ACCOUNT JSON FILE Path to Service Account JSON key file
CREDENTIALS_FILE = '/home/bee/Camera_GCS/microalgae-pics-3a919eb92020.json'

# CHANGE TO YOUR BUCKET NAME IN GCS
BUCKET_NAME = 'rpi-gcs-pics'