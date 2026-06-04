import config
import time
import os
import subprocess
import firebase_admin
from firebase_admin import credentials, firestore

from upload_to_gcs import upload_file_to_gcs

def capture_and_process(camera_doc_ref):
    # 1. Prepare Filename
    filename = time.strftime(f"{config.SAVE_PATH}%Y%m%d_%H%M%S.jpg")

    # 2. Optimized Command (Small file size, no preview)
    # Using -q 80 and 720p resolution as discussed
    command = [
        "rpicam-still",
        "-n",                  # No preview
        "-t", "1000",          # 1 second warmup
        "--width", "1280", 
        "--height", "720",
        "-q", "80",            # Compression quality
        "-o", filename
    ]

    try:
        # executes the command and waits for it to complete
        subprocess.run(command, check=True)
        print(f"Captured: {os.path.basename(filename)}")
        
        # 3. Upload to GCS
        success, public_url = upload_file_to_gcs(filename)

        # 4. Update Firestore: Reset trigger and update URL and latest picture upload time
        update_data = {
            "isCaptureRequested": False,
            "last_capture_time": firestore.SERVER_TIMESTAMP
        }
        if success:
            update_data["last_image_url"] = public_url
            print(f"Update successful. URL: {public_url}")
        
        camera_doc_ref.update(update_data)

    except Exception as e:
        print(f"Error: {e}")
        camera_doc_ref.update({"isCaptureRequested": False})
        
def start_camera_watcher(db_private, system_id):
    """Initializes the Firestore snapshot listener for the camera."""
    camera_doc_ref = db_private.collection("camera").document(system_id)

    def on_camera_trigger(doc_snapshot, changes, read_time):
        for doc in doc_snapshot:
            # Safety check: Ensure the document ID matches our physical hardware ID 
            if doc.id == system_id:
                data = doc.to_dict()
                
                # Check if the App has requested a new image 
                if data and data.get("isCaptureRequested") is True:
                    print(f"\n[!] Valid Camera Trigger for {system_id}")
                    # Executes the rpicam-still command and uploads to GCS 
                    capture_and_process(camera_doc_ref) 
            else:
                # Ignores updates meant for other systems (e.g., system_2)
                print(f"Ignoring trigger meant for {doc.id}")

    print(f"Starting Dedicated Camera Listener for {system_id}...")
    # Returns the watcher so it can be unsubscribed during shutdown in main.py 
    return camera_doc_ref.on_snapshot(on_camera_trigger)