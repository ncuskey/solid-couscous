
import base64
import os
import re

# Configuration
SOURCE_DIR = '/Users/nickcuskey/xmasLockboxes/www'
ASSETS_DIR = os.path.join(SOURCE_DIR, 'assets/toy_factory')
SOURCE_HTML = os.path.join(SOURCE_DIR, 'toy_factory.html')
OUTPUT_HTML = os.path.join(SOURCE_DIR, 'sam.html')

# Asset mappings (ID used in JS -> Filename)
GAME_ASSETS = {
    'CLAW': 'claw.png',
    # Toys
    'TOY_Car': 'Car.png',
    'TOY_Present': 'Present.png',
    'TOY_Robot': 'Robot.png',
    'TOY_Teddy': 'Teddy.png',
    'TOY_Top': 'Top.png',
    'TOY_Train': 'Train.png',
    # Tools/Junk
    'TOOL_Bolt': 'Bolt.png',
    'TOOL_Cog': 'Cog.png',
    'TOOL_Hammer': 'Hammer.png',
    'TOOL_Oil': 'Oil Can.png',
    'TOOL_Scrap': 'Scrap.png',
    'TOOL_Wrench': 'Wrench.png'
}

BACKGROUND_FILE = 'background.png'
MUSIC_FILE = 'Merry Merry Everywhere.mp3'

def get_b64_image(filepath):
    """Reads an image and returns a base64 data URI."""
    if not os.path.exists(filepath):
        print(f"Warning: File not found: {filepath}")
        return ""
    
    ext = os.path.splitext(filepath)[1].lower().replace('.', '')
    mime_type = f"image/{ext}"
    if ext == 'jpg': mime_type = 'image/jpeg'
    
    with open(filepath, 'rb') as f:
        data = base64.b64encode(f.read()).decode('utf-8')
        return f"data:{mime_type};base64,{data}"

def get_b64_audio(filepath):
    """Reads an audio file and returns a base64 data URI."""
    if not os.path.exists(filepath):
        print(f"Warning: File not found: {filepath}")
        return ""
    
    with open(filepath, 'rb') as f:
        data = base64.b64encode(f.read()).decode('utf-8')
        return f"data:audio/mp3;base64,{data}"

print("Building sam.html...")

# 1. Read Source HTML
with open(SOURCE_HTML, 'r') as f:
    html_content = f.read()

# 2. Prepare Asset Bundle
embedded_assets = {}

# Game Sprite Assets
# Game Sprite Assets
for asset_id, filename in GAME_ASSETS.items():
    path = os.path.join(ASSETS_DIR, filename)
    print(f"Embedding {filename}...")
    embedded_assets[asset_id] = get_b64_image(path)

# Music
music_path = os.path.join(SOURCE_DIR, MUSIC_FILE)
print(f"Embedding music: {MUSIC_FILE}...")
embedded_assets['MUSIC'] = get_b64_audio(music_path)

# Background
bg_path = os.path.join(ASSETS_DIR, BACKGROUND_FILE)
print(f"Embedding background: {BACKGROUND_FILE}...")
bg_b64 = get_b64_image(bg_path)

# Start Screen
start_bg_path = os.path.join(SOURCE_DIR, 'background_factory.jpg')
print(f"Embedding start background: background_factory.jpg...")
start_bg_b64 = get_b64_image(start_bg_path)
embedded_assets['START_BG'] = start_bg_b64


# 3. Inject Assets into HTML

# A. Insert the JS Asset Bundle
# We look for the <script> tag and insert our specific variable before it
# Or just insert it at the specific injection point we modified in the HTML?
# Actually, since we control the HTML, we can just prepend it to the first script tag 
# or inject it into the head.

# Create the JS object string
js_bundle = "window.EMBEDDED_ASSETS = {\n"
for key, val in embedded_assets.items():
    js_bundle += f"    '{key}': '{val}',\n"
js_bundle += "};\n"

# Inject into the <script> block. We'll look for `const canvas =` which starts the main logic
html_content = html_content.replace(
    "const canvas = document.getElementById('gameCanvas');", 
    f"{js_bundle}\n        const canvas = document.getElementById('gameCanvas');"
)

# B. Replace CSS Background
html_content = html_content.replace(
    "url('background.png')",
    f"url('{bg_b64}')"
)

# C. Clean up Audio Tag (Optional, since JS handles it, but good to be clean)
# <audio id="bgMusic" src="Merry Merry Everywhere.mp3" loop></audio>
# We can just empty the src since JS will fill it, or replace it.
# Let's replace it to be safe so it doesn't try to load the file.
html_content = html_content.replace(
    'src="Merry Merry Everywhere.mp3"',
    'src=""' # JS will populate this from EMBEDDED_ASSETS['MUSIC']
)

# 4. Write Output
with open(OUTPUT_HTML, 'w') as f:
    f.write(html_content)

print(f"Success! Generated {OUTPUT_HTML} ({len(html_content) / 1024 / 1024:.2f} MB)")
