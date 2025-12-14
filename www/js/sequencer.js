class ScriptSequencer {
    constructor() {
        this.script = [];
        this.currentIndex = -1;
        this.isPlaying = false;
        this.audioElement = new Audio();
        this.handlers = {
            onItemStart: null, // (item) => {}
            onItemEnd: null,   // (item) => {}
            onComplete: null   // () => {}
        };

        this.audioElement.addEventListener('ended', () => this.next());
        this.audioElement.addEventListener('error', (e) => {
            console.error("Audio Error:", e);
            setTimeout(() => this.next(), 1000);
        });

        // Analysis
        this.audioContext = null;
        this.analyser = null;
        this.dataArray = null;
        this.source = null;
    }

    initAudioContext() {
        if (this.audioContext) return;
        const AudioContext = window.AudioContext || window.webkitAudioContext;
        this.audioContext = new AudioContext();
        this.analyser = this.audioContext.createAnalyser();
        this.analyser.fftSize = 256;

        // Auto-leveling (Compression)
        this.compressor = this.audioContext.createDynamicsCompressor();
        this.compressor.threshold.setValueAtTime(-24, this.audioContext.currentTime);
        this.compressor.knee.setValueAtTime(30, this.audioContext.currentTime);
        this.compressor.ratio.setValueAtTime(12, this.audioContext.currentTime);
        this.compressor.attack.setValueAtTime(0.003, this.audioContext.currentTime);
        this.compressor.release.setValueAtTime(0.25, this.audioContext.currentTime);

        this.source = this.audioContext.createMediaElementSource(this.audioElement);
        this.source.connect(this.compressor);
        this.compressor.connect(this.analyser);
        this.analyser.connect(this.audioContext.destination);

        this.dataArray = new Uint8Array(this.analyser.frequencyBinCount);
    }

    getAmplitude() {
        if (!this.analyser) return 0;
        this.analyser.getByteFrequencyData(this.dataArray);
        let sum = 0;
        // Average the frequencies
        for (let i = 0; i < this.dataArray.length; i++) {
            sum += this.dataArray[i];
        }
        return sum / this.dataArray.length / 255; // Normalized 0.0 - 1.0
    }

    loadScript(script) {
        this.script = script; // Array of { character: "tink", file: "path/to.mp3", text: "..." }
        this.stop();
    }

    play() {
        if (this.script.length === 0) return;

        // Resume/Start Context
        if (!this.audioContext) this.initAudioContext();
        if (this.audioContext.state === 'suspended') this.audioContext.resume();

        this.isPlaying = true;

        // If restarting
        if (this.currentIndex === -1 || this.currentIndex >= this.script.length) {
            this.currentIndex = 0;
        }

        this.playCurrent();
    }

    stop() {
        this.isPlaying = false;
        this.currentIndex = -1;
        this.audioElement.pause();
        this.audioElement.currentTime = 0;
    }

    playCurrent() {
        if (!this.isPlaying) return;
        if (this.currentIndex >= this.script.length) {
            this.isPlaying = false;
            if (this.handlers.onComplete) this.handlers.onComplete();
            return;
        }

        const item = this.script[this.currentIndex];
        console.log(`Sequencer: Playing [${item.character}] ${item.file}`);

        // Trigger Start Event
        if (this.handlers.onItemStart) this.handlers.onItemStart(item);

        // Play Audio
        this.audioElement.src = item.file;
        this.audioElement.play().catch(e => {
            console.error("Play failed", e);
            // Advance anyway simulation
            setTimeout(() => this.next(), 2000);
        });
    }

    next() {
        if (!this.isPlaying) return;

        const currentItem = this.script[this.currentIndex];
        // Trigger End Event
        if (this.handlers.onItemEnd) this.handlers.onItemEnd(currentItem);

        // Advance
        this.currentIndex++;

        // Small delay between lines for realism?
        setTimeout(() => {
            this.playCurrent();
        }, 300);
    }

    // Event Registration
    on(event, callback) {
        if (this.handlers.hasOwnProperty(event)) {
            this.handlers[event] = callback;
        }
    }
}

// Make global
window.ScriptSequencer = ScriptSequencer;
