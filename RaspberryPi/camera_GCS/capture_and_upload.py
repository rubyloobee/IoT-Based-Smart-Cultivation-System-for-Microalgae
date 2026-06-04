import time
import os
import subprocess
import config
from upload_to_gcs import upload_file_to_gcs

# Define camera settings for rpicam-still
WIDTH = 1920
HEIGHT = 1080
QUALITY = 75


# The main capture loop
while True:
    # Create unique filename
    filename = time.strftime(f"{config.SAVE_PATH}%Y%m%d_%H%M%S.jpg")
    
    # Construct the rpicam-still command
    # -n: disables the preview window
    command = [
        "rpicam-still","-n",
        "--width", str(WIDTH), "--height", str(HEIGHT),
        "-q", str(QUALITY),
        "-o", filename,            # Output file path
    ]

    print(f"Executing: {' '.join(command)}")
    
    # The run function executes the command and waits for it to complete
    result = subprocess.run(command, check=True, capture_output=True, text=True)
    print(f"Captured: {filename}")
    
    # Immediatly upload the captured image to GCS
    print(f"Attempting to upload: {os.path.basename(filename)}")
    success, url = upload_file_to_gcs(filename)

    # Wait for the next interval
    time.sleep(config.INTERVAL_SECONDS)
    
    
    
    
    