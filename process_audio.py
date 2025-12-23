import os
import json
import speech_recognition as sr
from pydub import AudioSegment
from pydub.silence import split_on_silence

# Configuration
AUDIO_FILE = "Audio/ElevenLabs_2025-12-23T22_29_06_Flicker - Cheerful Fairy & Sparkly Sweetness_pvc_sp95_s0_sb100_v3.mp3"
OUTPUT_JSON = "transcript_chunks.json"
TEMP_DIR = "temp_chunks"

def process_audio():
    if not os.path.exists(AUDIO_FILE):
        print(f"File not found: {AUDIO_FILE}")
        return

    print("Loading audio...")
    sound = AudioSegment.from_mp3(AUDIO_FILE)
    
    print("Splitting on silence (this may take a moment)...")
    # Adjust silence threshold and length as needed
    chunks = split_on_silence(
        sound, 
        min_silence_len=700, 
        silence_thresh=sound.dBFS - 14,
        keep_silence=200
    )
    
    print(f"Found {len(chunks)} chunks.")
    
    if not os.path.exists(TEMP_DIR):
        os.makedirs(TEMP_DIR)
        
    recognizer = sr.Recognizer()
    results = []
    
    current_time = 0
    
    # We need to calculate timing roughly since split_on_silence loses absolute position unless we track it manually
    # Actually, split_on_silence DOES lose timing. pydub doesn't give it back easily.
    # We might need to implement a custom splitter or iterate differently to keep time.
    # For now, let's use a simpler approach: detect non-silent ranges manually to keep time.
    
    # Better approach: Use detect_nonsilent to get ranges, then extract.
    from pydub.silence import detect_nonsilent
    
    print("Detecting non-silent ranges...")
    ranges = detect_nonsilent(
        sound,
        min_silence_len=700,
        silence_thresh=sound.dBFS - 14
    )
    
    print(f"Detected {len(ranges)} non-silent ranges.")
    
    for i, (start_i, end_i) in enumerate(ranges):
        # Add padding
        start_ms = max(0, start_i - 200)
        end_ms = min(len(sound), end_i + 200)
        
        chunk = sound[start_ms:end_ms]
        chunk_path = os.path.join(TEMP_DIR, f"chunk_{i:03d}.wav")
        chunk.export(chunk_path, format="wav")
        
        # Transcribe
        text = ""
        try:
            with sr.AudioFile(chunk_path) as source:
                audio_data = recognizer.record(source)
                text = recognizer.recognize_google(audio_data)
                print(f"Chunk {i}: {text}")
        except sr.UnknownValueError:
            print(f"Chunk {i}: [Unintelligible]")
        except Exception as e:
            print(f"Chunk {i} Error: {e}")
            
        results.append({
            "id": i,
            "start_ms": start_ms,
            "end_ms": end_ms,
            "text": text,
            "filename": f"chunk_{i:03d}.wav"
        })
        
    with open(OUTPUT_JSON, "w") as f:
        json.dump(results, f, indent=2)
        
    print(f"Saved {len(results)} segments to {OUTPUT_JSON}")

if __name__ == "__main__":
    process_audio()
