
from PIL import Image
import os

assets_dir = '/Users/nickcuskey/xmasLockboxes/www/assets/toy_factory'

# Files to process
files = [
    'background.png', # Typically BG doesn't need transparency but good to standardize or maybe skip if full bleed
    'toys.png',
    'tools.png',
    'claw.png'
]

def add_transparency(img_path, is_claw=False):
    try:
        img = Image.open(img_path)
        img = img.convert("RGBA")
        datas = img.getdata()

        new_data = []
        for item in datas:
            r, g, b = item[0], item[1], item[2]
            
            # 1. Remove White (standard)
            if r > 240 and g > 240 and b > 240:
                new_data.append((255, 255, 255, 0))
                continue
            
            # 2. (Removed legacy checkerboard logic for old claw asset)
            # The new claw asset interacts well with standard white removal.
            pass
            
            new_data.append(item)

        img.putdata(new_data)
        img.save(img_path, "PNG")
        print(f"Processed transparency for {img_path}")
    except Exception as e:
        print(f"Failed to process {img_path}: {e}")

# First, let's make sure we copy the new spritesheet to the assets dir locally
# (This part is handled by the agent manually, but assuming it sits in artifacts)
# This script assumes files are already in assets_dir

for filename in files:
    full_path = os.path.join(assets_dir, filename)
    if os.path.exists(full_path):
        # Skip background transparency usually
        if 'background' not in filename: 
             add_transparency(full_path, is_claw=(filename == 'claw.png'))
    else:
        print(f"File not found: {full_path}")
