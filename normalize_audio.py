import os
import subprocess

AUDIO_DIR = "www/audio"
TEMP_DIR = "www/audio_norm"

if not os.path.exists(TEMP_DIR):
    os.makedirs(TEMP_DIR)

for filename in os.listdir(AUDIO_DIR):
    if filename.endswith(".mp3"):
        in_path = os.path.join(AUDIO_DIR, filename)
        out_path = os.path.join(TEMP_DIR, filename)
        
        # Expert DSP Chain:
        # 1. HighPass @ 250Hz (Kill power-wasting bass)
        # 2. EQ: +2dB @ 850Hz, +4dB @ 3kHz, +2dB @ 6kHz (Vocal Presence)
        # 3. Compression (Control dynamics)
        # 4. Loudness Normalization (Target -14 LUFS)
        
        filter_chain = (
            "highpass=f=250,"
            "equalizer=f=850:width_type=o:width=1:g=2,"
            "equalizer=f=3000:width_type=o:width=1:g=4,"
            "equalizer=f=6000:width_type=o:width=1:g=2,"
            "compand=attacks=0.01:decays=0.1:points=-80/-80|-15/-15|0/-3:gain=0," 
            "loudnorm=I=-14:TP=-1.0:LRA=11"
        )

        cmd = [
            "ffmpeg", "-y", "-i", in_path,
            "-filter:a", filter_chain,
            "-ar", "44100", # Ensure standard rate
            out_path
        ]
        
        print(f"Processing {filename}...")
        try:
            subprocess.run(cmd, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        except subprocess.CalledProcessError:
            print(f"Failed to normalize {filename}")

print("Done. Normalized files in www/audio_norm")
