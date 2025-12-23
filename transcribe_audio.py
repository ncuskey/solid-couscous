import whisper
import json
import os

AUDIO_FILE = "Audio/ElevenLabs_2025-12-23T22_29_06_Flicker - Cheerful Fairy & Sparkly Sweetness_pvc_sp95_s0_sb100_v3.mp3"
OUTPUT_FILE = "transcript.json"

def transcribe():
    if not os.path.exists(AUDIO_FILE):
        print(f"Error: Audio file not found at {AUDIO_FILE}")
        return

    print("Loading model...")
    model = whisper.load_model("base") # Use base model for speed/accuracy trade-off
    
    print(f"Transcribing {AUDIO_FILE}...")
    result = model.transcribe(AUDIO_FILE)
    
    # Save detailed segments
    with open(OUTPUT_FILE, "w") as f:
        json.dump(result["segments"], f, indent=2)
    
    print(f"Transcription complete. Saved to {OUTPUT_FILE}")
    
    # Print for immediate view
    for segment in result["segments"]:
        print(f"[{segment['start']:.2f} - {segment['end']:.2f}] {segment['text']}")

if __name__ == "__main__":
    transcribe()
