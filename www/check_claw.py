
from PIL import Image
import os

path = '/Users/nickcuskey/xmasLockboxes/www/assets/toy_factory/claw.png'

try:
    img = Image.open(path)
    if img.mode in ('RGBA', 'LA') or (img.mode == 'P' and 'transparency' in img.info):
        # Check if it actually has transparent pixels
        extrema = img.getextrema()
        if img.mode == 'RGBA':
            alpha_extrema = extrema[3]
            if alpha_extrema[0] < 255:
                print("Yes, the claw background is transparent (Alpha channel present and used).")
            else:
                print("The image has an Alpha channel but it is fully opaque.")
        else:
             print("Image mode supports transparency.")
    else:
        print("No, the claw background is NOT transparent (No Alpha channel).")
except Exception as e:
    print(f"Error checking image: {e}")
