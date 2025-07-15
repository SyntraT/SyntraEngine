import os
import subprocess
import sys

# Install gdown if not already installed
try:
    import gdown
except ImportError:
    print("gdown module not found. Installing...")
    subprocess.check_call([sys.executable, "-m", "pip", "install", "gdown"])
    import gdown

# Google Drive file ID
file_id = '1PQ9o0jPhN8bH-9-bxtOsC1CiHreMruDq'

# Construct download URL
url = f'https://drive.google.com/uc?id={file_id}'

# Destination file path (same folder as this script)
current_dir = os.getcwd()
output_path = os.path.join(current_dir, 'models.rar')

# Download the file
gdown.download(url, output_path, quiet=False)

print(f"Download completed. File saved to: {output_path}")
