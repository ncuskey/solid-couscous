
from PIL import Image
import os

files = ['toys.png', 'tools.png', 'background.png']
base_dir = '/Users/nickcuskey/xmasLockboxes/www/assets/toy_factory'

for f in files:
    path = os.path.join(base_dir, f)
    if os.path.exists(path):
        with Image.open(path) as img:
            print(f"{f}: {img.size} (Width x Height)")
    else:
        print(f"{f}: Not found")
