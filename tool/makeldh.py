import re
import sys
import subprocess

# Generates an .ld and .h file from an ZeldaRet OoT decomp .map 

if len(sys.argv) < 4:
    print("Usage: python generate_ld.py <input.map> <suffix> <output_prefix>")
    sys.exit(1)

map_file_path = sys.argv[1]
suffix = sys.argv[2]
output_prefix = sys.argv[3]

ld_file_path = f"{output_prefix}.ld"
header_file_path = f"{output_prefix}.h"

symbol_re = re.compile(r'^\s*(0x[0-9a-fA-F]+)\s+(\S+)')

symbols = []
seen_symbols = set()

with open(map_file_path, "r") as f:
    for line in f:
        match = symbol_re.match(line)
        if match:
            addr_str, symbol = match.groups()
            
            if symbol.startswith('.'):
                continue
                
            if symbol in seen_symbols:
                continue

            addr = int(addr_str, 16) & 0xFFFFFFFF
           
            seen_symbols.add(symbol)
            
            symbol_with_suffix = f"{symbol}{suffix}"
            symbols.append((symbol_with_suffix, addr))


with open(ld_file_path, "w") as f:
    for sym, addr in symbols:
        f.write(f"{sym} = 0x{addr:08X};\n")

awk_cmd = f"""awk '/^[[:space:]]*[_A-Za-z]/ {{ gsub(/;/,"",$0); split($0,a,"="); gsub(/^[ \t]+|[ \t]+$/,"",a[1]); gsub(/^[ \t]+|[ \t]+$/,"",a[2]); print "#define " a[1] "  " a[2] }}' {ld_file_path} > {header_file_path}"""
subprocess.run(awk_cmd, shell=True, check=True)

print(f"Done.")
