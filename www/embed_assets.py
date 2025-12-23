
import base64
import os

html_path = '/Users/nickcuskey/xmasLockboxes/www/elf_game.html'
assets_dir = '/Users/nickcuskey/xmasLockboxes/www/assets/elf_game'

assets = {
    'background.png': 'assets/elf_game/background.png',
    'elf.png': 'assets/elf_game/elf.png',
    'present.png': 'assets/elf_game/present.png',
    'coal.png': 'assets/elf_game/coal.png'
}

with open(html_path, 'r') as f:
    content = f.read()

for filename, placeholder in assets.items():
    file_path = os.path.join(assets_dir, filename)
    with open(file_path, 'rb') as img_file:
        b64_data = base64.b64encode(img_file.read()).decode('utf-8')
        data_uri = f"data:image/png;base64,{b64_data}"
        content = content.replace(placeholder, data_uri)

with open(html_path, 'w') as f:
    f.write(content)

print("Assets embedded successfully.")
