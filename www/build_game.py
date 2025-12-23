
import base64
import os

assets_dir = '/Users/nickcuskey/xmasLockboxes/www/assets/elf_game'
output_path = '/Users/nickcuskey/xmasLockboxes/www/elf_game.html'

def get_b64(filename):
    path = os.path.join(assets_dir, filename)
    with open(path, 'rb') as f:
        return base64.b64encode(f.read()).decode('utf-8')

# Load assets
elf_sheet_b64 = get_b64('elf_spritesheet.png')
# Use the old static image if you want an idle frame, or just use frame 0 of sheet
present_b64 = get_b64('present.png')
coal_b64 = get_b64('coal.png')
bg_b64 = get_b64('background.png')

html_content = f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Christmas Elf Adventure</title>
    <script src="https://cdn.jsdelivr.net/npm/phaser@3.60.0/dist/phaser-arcade-physics.min.js"></script>
    <style>
        body {{
            margin: 0;
            padding: 0;
            background-color: #2e3b4e;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            overflow: hidden;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
        }}
        #game-container {{
            box-shadow: 0 0 50px rgba(0,0,0,0.5);
            border-radius: 10px;
            overflow: hidden;
        }}
    </style>
</head>
<body>
    <!-- Hidden Assets for direct loading -->
    <div style="display:none;">
        <img id="asset_elf_sheet" src="data:image/png;base64,{elf_sheet_b64}" />
        <img id="asset_present" src="data:image/png;base64,{present_b64}" />
        <img id="asset_coal" src="data:image/png;base64,{coal_b64}" />
        <img id="asset_background" src="data:image/png;base64,{bg_b64}" />
    </div>

    <div id="game-container"></div>

    <script>
        window.onload = function() {{
            const config = {{
                type: Phaser.AUTO,
                width: 800,
                height: 600,
                parent: 'game-container',
                physics: {{
                    default: 'arcade',
                    arcade: {{
                        gravity: {{ y: 600 }},
                        debug: false
                    }}
                }},
                scene: {{
                    preload: preload,
                    create: create,
                    update: update
                }}
            }};

            const game = new Phaser.Game(config);
        }};

        let player;
        let cursors;
        let presents;
        let coals;
        let score = 0;
        let scoreText;
        let gameOver = false;
        let background;

        function preload() {{
            // Assets are loaded via DOM
        }}

        function create() {{
            // Import textures from hidden DOM elements
            this.textures.addImage('background', document.getElementById('asset_background'));
            this.textures.addImage('present', document.getElementById('asset_present'));
            this.textures.addImage('coal', document.getElementById('asset_coal'));
            
            // For the elf, we need to add it as a spritesheet manually from the image
            const elfImg = document.getElementById('asset_elf_sheet');
            // Assuming 4 frames horizontally. We calculate frame width dynamically.
            const frameWidth = elfImg.width / 4; 
            const frameHeight = elfImg.height;

            this.textures.addSpriteSheet('elf_anim', elfImg, {{ 
                frameWidth: frameWidth, 
                frameHeight: frameHeight 
            }});

            // Background
            background = this.add.image(400, 300, 'background');
            background.setDisplaySize(800, 600);

            // Ground (Invisible)
            const ground = this.physics.add.staticGroup();
            const groundRect = this.add.rectangle(400, 580, 800, 40, 0x000000, 0);
            ground.add(groundRect);

            // Player - start with frame 0
            player = this.physics.add.sprite(100, 450, 'elf_anim', 0);
            player.setBounce(0.1);
            player.setCollideWorldBounds(true);
            player.setScale(0.5); // Adjust based on new sprite size. 
            
            // Adjust body size for better collision
            player.body.setSize(player.width * 0.4, player.height * 0.8);
            player.body.setOffset(player.width * 0.3, player.height * 0.1);

            // Animations
            this.anims.create({{
                key: 'run',
                frames: this.anims.generateFrameNumbers('elf_anim', {{ start: 0, end: 3 }}),
                frameRate: 10,
                repeat: -1
            }});

            this.anims.create({{
                key: 'idle',
                frames: [ {{ key: 'elf_anim', frame: 0 }} ],
                frameRate: 20
            }});

            // Physics
            this.physics.add.collider(player, ground);

            // Groups
            presents = this.physics.add.group();
            coals = this.physics.add.group();

            // Inputs
            cursors = this.input.keyboard.createCursorKeys();

            // UI
            scoreText = this.add.text(16, 16, 'Presents: 0', {{ 
                fontSize: '32px', 
                fill: '#fff',
                fontStyle: 'bold',
                backgroundColor: 'rgba(0,0,0,0.3)',
                padding: {{ x: 10, y: 5 }}
            }});

            // Spawning Timers
            this.time.addEvent({{
                delay: 2000,
                callback: spawnPresent,
                callbackScope: this,
                loop: true
            }});

            this.time.addEvent({{
                delay: 1500,
                callback: spawnCoal,
                callbackScope: this,
                loop: true
            }});

            // Collisions
            this.physics.add.overlap(player, presents, collectPresent, null, this);
            this.physics.add.collider(player, coals, hitCoal, null, this);
        }}

        function update() {{
            if (gameOver) return;

            if (cursors.left.isDown) {{
                player.setVelocityX(-300);
                player.flipX = true;
                player.anims.play('run', true);
            }} else if (cursors.right.isDown) {{
                player.setVelocityX(300);
                player.flipX = false;
                player.anims.play('run', true);
            }} else {{
                player.setVelocityX(0);
                player.anims.play('idle', true);
            }}

            if ((cursors.up.isDown || cursors.space.isDown) && player.body.touching.down) {{
                player.setVelocityY(-450);
            }}
        }}

        function spawnPresent() {{
            if (gameOver) return;
            const x = Phaser.Math.Between(50, 750);
            const present = presents.create(x, 0, 'present');
            present.setScale(0.1);
            present.setBounce(0.4);
            present.setCollideWorldBounds(false);
            present.setVelocityY(Phaser.Math.Between(100, 200));
        }}

        function spawnCoal() {{
            if (gameOver) return;
            const x = Phaser.Math.Between(50, 750);
            const coal = coals.create(x, 0, 'coal');
            coal.setScale(0.1);
            coal.setVelocityY(Phaser.Math.Between(200, 400));
            coal.setAngularVelocity(Phaser.Math.Between(-100, 100));
        }}

        function collectPresent(player, present) {{
            present.disableBody(true, true);
            score += 1;
            scoreText.setText('Presents: ' + score);
            this.cameras.main.flash(100, 255, 255, 255, false);
        }}

        function hitCoal(player, coal) {{
            this.physics.pause();
            player.setTint(0xff0000);
            player.anims.stop();
            gameOver = true;

            const gameOverText = this.add.text(400, 300, 'GAME OVER\\nClick to Restart', {{ 
                fontSize: '64px', 
                fill: '#ff0000', 
                align: 'center',
                fontStyle: 'bold',
                stroke: '#000',
                strokeThickness: 6
            }});
            gameOverText.setOrigin(0.5);

            this.input.on('pointerdown', () => {{
                score = 0;
                gameOver = false;
                this.scene.restart();
            }});
        }}
    </script>
</body>
</html>"""

with open(output_path, 'w') as f:
    f.write(html_content)

print(f"Rebuilt {output_path} with transparent animations.")
