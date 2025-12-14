/**
 * Text-to-Speech Manager using Web Speech API
 */
console.log("!!! TTS SCRIPT EXECUTING !!!");
class TTSManager {
    constructor() {
        this.synth = window.speechSynthesis;
        this.voices = [];
        this.enabled = true;
        this.queue = [];
        this.isSpeaking = false;

        // Wait for voices to be loaded
        if (speechSynthesis.onvoiceschanged !== undefined) {
            speechSynthesis.onvoiceschanged = () => this.loadVoices();
        }
        this.loadVoices();
    }

    loadVoices() {
        this.voices = this.synth.getVoices();
        console.log("TTS: Loaded " + this.voices.length + " voices.");
    }

    /**
     * Speak a text string with character context
     * @param {string} text - Text to speak
     * @param {string} character - 'jacob', 'kristine', 'sam', 'system'
     * @param {object} options - Overrides { rate, pitch, volume }
     */
    speak(text, character = 'system', options = {}) {
        if (!this.enabled || !text) return;

        this.queue.push({ text, character, options });
        this.processQueue();
    }

    processQueue() {
        if (this.isSpeaking || this.queue.length === 0) return;

        const item = this.queue.shift();
        this.isSpeaking = true;

        const utterance = new SpeechSynthesisUtterance(item.text);

        // 1. SELECT VOICE BASED ON CHARACTER
        // Adjust these keywords based on available OS voices (e.g. Mac vs Win)
        let voiceName = null;
        let pitch = 1.0;
        let rate = 1.1;

        switch (item.character.toLowerCase()) {
            case 'jacob':
                // Male, slightly lower, techy?
                // Mac: "Daniel", "Fred"? Win: "David"
                voiceName = "Daniel";
                if (!this.findVoice(voiceName)) voiceName = "David";
                pitch = 1.0;
                break;
            case 'kristine':
                // Female, higher
                // Mac: "Tessa", "Samantha"? Win: "Zira"
                voiceName = "Tessa";
                if (!this.findVoice(voiceName)) voiceName = "Zira";
                pitch = 1.1;
                break;
            case 'sam':
                // Youthful? Random?
                voiceName = "Fred"; // Mac specific robot?
                if (!this.findVoice(voiceName)) voiceName = "Mark";
                pitch = 1.2;
                break;
            default:
                voiceName = "Samantha";
        }

        const selectedVoice = this.findVoice(voiceName) || this.voices[0];
        if (selectedVoice) utterance.voice = selectedVoice;

        utterance.pitch = item.options.pitch || pitch;
        utterance.rate = item.options.rate || rate;
        utterance.volume = item.options.volume || 1.0;

        // 2. TRIGGER HARDWARE START
        console.log(`TTS Start: [${item.character}] "${item.text}"`);
        if (item.character !== 'system' && window.Network) {
            // Send to specific box topic: lockbox/<char>/cmd
            // We need to know which box maps to which character.
            // Assuming 1=Sam, 2=Kris, 3=Jacob based on progress indices in firmware?
            // Actually firmware uses "lockbox/1/cmd". We need mapping.
            // Let's assume shared.js or config.js handles name->id mapping later.
            // For now, let's pass the Name and let Network handle it.
            Network.sendAnim(item.character, 'speaking', 'on');
        }

        utterance.onend = () => {
            console.log(`TTS End: [${item.character}]`);
            if (item.character !== 'system' && window.Network) {
                Network.sendAnim(item.character, 'speaking', 'off');
            }
            this.isSpeaking = false;
            // Short pause between separate utterances
            setTimeout(() => this.processQueue(), 250);
        };

        utterance.onerror = (e) => {
            console.error("TTS Error:", e);
            if (item.character !== 'system' && window.Network) {
                Network.sendAnim(item.character, 'speaking', 'off'); // Safety off
            }
            this.isSpeaking = false;
            this.processQueue();
        };

        this.synth.speak(utterance);
    }

    findVoice(name) {
        if (!name) return null;
        return this.voices.find(v => v.name.includes(name));
    }

    stop() {
        this.synth.cancel();
        this.queue = [];
        this.isSpeaking = false;
    }
}

// Global instance
window.TTS = new TTSManager();
