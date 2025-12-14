/**
 * Shared Utilities for North Pole Apps
 */

// --- AUDIO MANAGEMENT ---
window.AudioManager = {
    bgmIds: [],

    init(audioElementId) {
        const audio = document.getElementById(audioElementId);
        if (!audio) return;

        // Auto-play attempt on interaction
        document.body.addEventListener('click', () => {
            if (audio.paused) {
                audio.volume = 0;
                audio.play().then(() => this.fadeIn(audio));
            }
        }, { once: true });

        // Setup toggle button
        const toggle = document.getElementById('musicToggle');
        if (toggle) {
            toggle.onclick = () => {
                if (audio.paused) {
                    audio.volume = 0;
                    audio.play();
                    this.fadeIn(audio);
                    toggle.innerText = '🔊';
                    toggle.style.opacity = 1;
                } else {
                    this.fadeOut(audio, () => audio.pause());
                    toggle.innerText = '🔇';
                    toggle.style.opacity = 0.5;
                }
            };
        }
    },

    fadeIn(audio, duration = 1000) {
        let vol = 0;
        const interval = 50;
        const step = 1 / (duration / interval);
        const fade = setInterval(() => {
            vol += step;
            if (vol >= 1) {
                vol = 1;
                clearInterval(fade);
            }
            audio.volume = vol;
        }, interval);
    },

    fadeOut(audio, callback, duration = 500) {
        let vol = audio.volume;
        const interval = 50;
        const step = vol / (duration / interval);
        const fade = setInterval(() => {
            vol -= step;
            if (vol <= 0) {
                vol = 0;
                clearInterval(fade);
                if (callback) callback();
            }
            audio.volume = vol;
        }, interval);
    }
};

// --- SNOWFALL EFFECT ---
window.Snowfall = {
    init() {
        // Simple CSS-based particle generator could go here
        // or we just rely on a CSS background.
        // Let's do a lightweight canvas snow for high production value.
        const canvas = document.createElement('canvas');
        canvas.style.position = 'fixed';
        canvas.style.top = '0';
        canvas.style.left = '0';
        canvas.style.width = '100%';
        canvas.style.height = '100%';
        canvas.style.pointerEvents = 'none';
        canvas.style.zIndex = '-1';
        document.body.appendChild(canvas);

        const ctx = canvas.getContext('2d');
        let width = canvas.width = window.innerWidth;
        let height = canvas.height = window.innerHeight;

        const particles = [];
        const count = 50;

        for (let i = 0; i < count; i++) {
            particles.push({
                x: Math.random() * width,
                y: Math.random() * height,
                r: Math.random() * 3 + 1,
                d: Math.random() * count
            });
        }

        function draw() {
            ctx.clearRect(0, 0, width, height);
            ctx.fillStyle = "rgba(255, 255, 255, 0.6)";
            ctx.beginPath();
            for (let i = 0; i < count; i++) {
                let p = particles[i];
                ctx.moveTo(p.x, p.y);
                ctx.arc(p.x, p.y, p.r, 0, Math.PI * 2, true);
            }
            ctx.fill();
            update();
            requestAnimationFrame(draw);
        }

        let angle = 0;
        function update() {
            angle += 0.01;
            for (let i = 0; i < count; i++) {
                let p = particles[i];
                p.y += Math.cos(angle + p.d) + 1 + p.r / 2;
                p.x += Math.sin(angle) * 2;

                if (p.x > width + 5 || p.x < -5 || p.y > height) {
                    if (i % 3 > 0) {
                        particles[i] = { x: Math.random() * width, y: -10, r: p.r, d: p.d };
                    } else {
                        if (Math.sin(angle) > 0) {
                            particles[i] = { x: -5, y: Math.random() * height, r: p.r, d: p.d };
                        } else {
                            particles[i] = { x: width + 5, y: Math.random() * height, r: p.r, d: p.d };
                        }
                    }
                }
            }
        }

        window.addEventListener('resize', () => {
            width = canvas.width = window.innerWidth;
            height = canvas.height = window.innerHeight;
        });

        draw();
    }
};

// Auto-init specific features if requested
window.addEventListener('DOMContentLoaded', () => {
    if (document.body && document.body.getAttribute('data-snow') === 'true') {
        Snowfall.init();
    }
});
// --- MQTT NETWORK MANAGER ---
window.Network = {
    client: null,
    status: {},
    onStatusChange: null,

    init() {
        console.log("Connecting to MQTT...");
        this.client = new Paho.MQTT.Client(
            CONFIG.MQTT.HOST,
            Number(CONFIG.MQTT.PORT),
            CONFIG.MQTT.CLIENT_ID
        );

        this.client.onConnectionLost = (responseObject) => {
            if (responseObject.errorCode !== 0) {
                console.log("onConnectionLost:" + responseObject.errorMessage);
                // Auto reconnect after 5s
                setTimeout(() => this.connect(), 5000);
            }
        };

        this.client.onMessageArrived = (message) => {
            console.log("Msg Arrived: " + message.payloadString);
            try {
                const data = JSON.parse(message.payloadString);
                this.status = data;
                if (this.onStatusChange) this.onStatusChange(data);
            } catch (e) { console.error("JSON Parse Error", e); }
        };

        this.connect();
    },

    connect() {
        this.client.connect({
            onSuccess: () => {
                console.log("MQTT Connected");
                // Default sub to first box for simple pages
                if (CONFIG.BOXES && CONFIG.BOXES.length > 0) {
                    this.client.subscribe(CONFIG.BOXES[0].topic + "/status");
                }
            },
            onFailure: (e) => {
                console.log("MQTT Fail", e);
                setTimeout(() => this.connect(), 5000);
            },
            useSSL: false // Set to true if using wss://, but our server config is likely just ws:// for now
        });
    },

    sendAnim(character, type, state) {
        // Map character to box index
        let boxIndex = 0;
        if (typeof character === 'string') {
            const charLower = character.toLowerCase();
            // Assuming CONFIG.BOXES is ordered [Sam, Kris, Jacob] or similar
            // We need a way to map names to indices. 
            // Let's assume a convention or just use topic search.
            // For now, based on firmware logic:
            // Box 1 (index 0) seems to be the main controller or at least the one we connect to.
            // Wait, the user said "Each box needs a different voice... their box should have light animation".
            // This implies MULTIPLE PHYSICAL BOXES listening to different topics (e.g., lockbox/1, lockbox/2, lockbox/3).
            // But the current config in shared.js (lines 192-210) defaults to boxIndex=0.

            // NOTE: We need to see config.js to know if multiple boxes are defined.
            // I'll add a simple lookup helper.
            boxIndex = this.getBoxIndex(charLower);
        }

        this.send('anim', { type: type, state: state }, boxIndex);
    },

    getBoxIndex(name) {
        if (!CONFIG.BOXES) {
            console.error("CONFIG.BOXES is undefined!");
            return 0;
        }
        const search = name.toLowerCase();
        console.log(`Debug getBoxIndex: searching for '${search}' in`, CONFIG.BOXES);

        // Try exact match on ID first
        let idx = CONFIG.BOXES.findIndex(b => b.id && b.id.toLowerCase() === search);
        if (idx >= 0) {
            console.log(`Found exact match at index ${idx}`);
            return idx;
        }

        // Try includes on ID
        idx = CONFIG.BOXES.findIndex(b => b.id && b.id.toLowerCase().includes(search));
        if (idx >= 0) {
            console.log(`Found partial ID match at index ${idx}`);
            return idx;
        }

        // Try includes on Name
        idx = CONFIG.BOXES.findIndex(b => b.name && b.name.toLowerCase().includes(search));
        if (idx >= 0) {
            console.log(`Found partial Name match at index ${idx}`);
            return idx;
        }

        console.warn(`No match found for '${search}', defaulting to 0`);
        return 0; // Fallback to Box 1
    },

    send(action, payload = {}, boxIndex = 0) {
        if (!this.client || !this.client.isConnected()) return;
        if (!CONFIG.BOXES || !CONFIG.BOXES[boxIndex]) return;

        const msgData = { action: action, ...payload };
        const message = new Paho.MQTT.Message(JSON.stringify(msgData));
        message.destinationName = CONFIG.BOXES[boxIndex].topic + "/cmd";
        this.client.send(message);
    },

    // Convenience Wrappers
    sendSolve(kid) { this.send('solve', { kid: kid }); },
    sendReady(kid, ready) { this.send('finale_ready', { kid: kid, ready: ready }); },
    sendUnlock() { this.send('unlock'); },
    sendLock() { this.send('lock'); },
    sendReset() { this.send('reset'); }
};

// Initializer
if (typeof Paho !== 'undefined') {
    Network.init();
} else {
    // Wait for script load if needed? 
    // Usually scripts load sequentially.
    window.addEventListener('load', () => {
        if (typeof Paho !== 'undefined') Network.init();
    });
}
