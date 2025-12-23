import os
import glob

def convert_audio():
    # Target directory with MP3s
    source_dir = "Audio"
    output_header = "audio_assets.h"
    
    print(f"Scanning {source_dir}...")
    files = glob.glob(os.path.join(source_dir, "*.mp3"))
    
    with open(output_header, "w") as out:
        out.write("#ifndef AUDIO_ASSETS_H\n")
        out.write("#define AUDIO_ASSETS_H\n\n")
        out.write("#include <pgmspace.h>\n\n")
        
        for fpath in files:
            fname = os.path.basename(fpath)
            var_name = fname.replace(" ", "_").replace(".", "_").lower()
            
            print(f"Processing {fname}...")
            
            with open(fpath, "rb") as f:
                data = f.read()
                
            out.write(f"// {fname} ({len(data)} bytes)\n")
            out.write(f"const uint8_t {var_name}[] PROGMEM = {{\n")
            
            # Write hex data
            for i, byte in enumerate(data):
                if i % 16 == 0:
                    out.write("  ")
                out.write(f"0x{byte:02x},")
                if i % 16 == 15:
                    out.write("\n")
            
            out.write("\n};\n")
            out.write(f"const unsigned int {var_name}_len = {len(data)};\n\n")
            
        out.write("#endif\n")
        
    print(f"Done! Generated {output_header}")

if __name__ == "__main__":
    convert_audio()
