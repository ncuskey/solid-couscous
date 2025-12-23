from pydub import AudioSegment
import json
import os
import re

# CONFIG
SOURCE_AUDIO = "Audio/ElevenLabs_2025-12-23T22_29_06_Flicker - Cheerful Fairy & Sparkly Sweetness_pvc_sp95_s0_sb100_v3.mp3"
TRANSCRIPT_FILE = "manual_transcript.txt"
OUTPUT_DIR = "www/audio"
SEQUENCE_FILE = "www/show_sequence.json"

# MAPPING
# Box1: Sam (Shiny Buttons)
# Box2: Jacob (Timestamp)
# Box3: Kristine (Patiently)
BOX_MAP = {
    "Box1": "kristine",
    "Box2": "jacob",
    "Box3": "sam"
}

def parse_time(t_str):
    # Format: 00:00:00,340
    # H:M:S,ms
    hours, mins, sec_ms = t_str.split(':')
    secs, ms = sec_ms.split(',')
    
    total_ms = (int(hours) * 3600000) + \
               (int(mins) * 60000) + \
               (int(secs) * 1000) + \
               int(ms)
    return total_ms

def parse_transcript():
    segments = []
    with open(TRANSCRIPT_FILE, 'r') as f:
        content = f.read()
        
    # Split by double newline or block
    blocks = content.strip().split('\n\n')
    
    for block in blocks:
        lines = block.strip().split('\n')
        if len(lines) < 2: continue
        
        # Line 1: Timestamp [Box]
        # 00:00:00,340 --> 00:00:03,119 [Box1]
        header = lines[0]
        match = re.search(r'(\d{2}:\d{2}:\d{2},\d{3}) --> (\d{2}:\d{2}:\d{2},\d{3})\s+\[(.*?)\]', header)
        if match:
            start_str = match.group(1)
            end_str = match.group(2)
            box_id = match.group(3)
            
            # Line 2+: Text
            text = " ".join(lines[1:])
            
            segments.append({
                "start": parse_time(start_str),
                "end": parse_time(end_str),
                "box": BOX_MAP.get(box_id, "unknown"),
                "text": text
            })
    return segments

def generate_assets():
    print(f"Loading {SOURCE_AUDIO}...")
    original = AudioSegment.from_mp3(SOURCE_AUDIO)
    
    # Create silent base tracks
    duration_ms = len(original)
    silence = AudioSegment.silent(duration=duration_ms)
    
    tracks = {
        "sam": silence,
        "kristine": silence,
        "jacob": silence
    }
    
    lighting_events = []
    
    print("Parsing transcript...")
    segments = parse_transcript()
    print(f"Found {len(segments)} segments.")
    
    for seg in segments:
        speaker = seg["box"]
        start = seg["start"]
        end = seg["end"]
        
        if speaker not in tracks:
            print(f"Warning: Unknown speaker {speaker}")
            continue
            
        print(f"Processing {speaker} ({start}-{end}): {seg['text'][:20]}...")
        
        # Audio Processing
        # Extract chunk from original
        # Safety clip
        end = min(end, len(original))
        audio_chunk = original[start:end]
        
        # Overlay onto specific track
        tracks[speaker] = tracks[speaker].overlay(audio_chunk, position=start)
        
        # Lighting Events
        lighting_events.append({
            "time": start,
            "box": speaker,
            "action": "anim",
            "type": "speaking",
            "state": "on"
        })
        lighting_events.append({
            "time": end,
            "box": speaker,
            "action": "anim",
            "type": "speaking",
            "state": "off"
        })
        
    # Sort events
    lighting_events.sort(key=lambda x: x["time"])
    
    # Export Audio
    if not os.path.exists(OUTPUT_DIR):
        os.makedirs(OUTPUT_DIR)
        
    for name, audio in tracks.items():
        filename = f"Finale_{name.capitalize()}.mp3"
        path = os.path.join(OUTPUT_DIR, filename)
        print(f"Exporting {filename}...")
        audio.export(path, format="mp3", bitrate="128k")
        
    # Export Sequence
    with open(SEQUENCE_FILE, "w") as f:
        json.dump(lighting_events, f, indent=2)
    print(f"Sequence saved to {SEQUENCE_FILE}")

if __name__ == "__main__":
    generate_assets()
