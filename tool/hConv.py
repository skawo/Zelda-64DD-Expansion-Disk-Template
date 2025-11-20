#!/usr/bin/env python3
import os
import argparse

def sanitize_symbol(name: str) -> str:
    name = name.upper()
    allowed = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
    result = []
    last_was_underscore = False
    for ch in name:
        if ch in allowed:
            result.append(ch)
            last_was_underscore = False
        else:
            if not last_was_underscore:
                result.append("_")
                last_was_underscore = True

    sym = "".join(result).strip("_")
    if sym and sym[0].isdigit():
        sym = "_" + sym

    return sym



def process_file(file_path, asm_file, header_file):
    file_name = os.path.basename(file_path)
    symbol = sanitize_symbol(file_name)

    with open(file_path, "rb") as f:
        data = f.read()

    # Write ASM
    with open(asm_file, "a") as asm_f:
        asm_f.write(f".global {symbol}\n")
        asm_f.write(f"{symbol}:\n")
        byte_chunks = [f"0x{b:02x}" for b in data]
        for i in range(0, len(byte_chunks), 16):
            asm_f.write("    .byte " + ", ".join(byte_chunks[i:i+16]) + "\n")
        asm_f.write("\n")  # No _LEN symbol here

    # Write header
    with open(header_file, "a") as header_f:
        header_f.write(f"extern unsigned char {symbol}[];\n")
        header_f.write(f"#define {symbol}_LEN {len(data)}\n\n")


def main():
    parser = argparse.ArgumentParser(description="Generate ASM and header from filesystem files.")
    parser.add_argument("input_folder", help="Path to the input folder containing files")
    parser.add_argument("output_folder", help="Path to the output folder")
    parser.add_argument("--priority", help="Filename (relative to input folder) to place first", default=None)
    parser.add_argument("--extensions", help="Comma-separated list of file extensions to include",
                        default="zmap,zscene,bin,yaz0,tbl")
    args = parser.parse_args()

    src_dir = args.input_folder
    out_dir = args.output_folder
    header_file = os.path.join(out_dir, "filesystem.h")
    asm_file = os.path.join(out_dir, "filesystem.S")
    priority_file = args.priority
    extensions = set(ext.strip().lower() for ext in args.extensions.split(","))

    os.makedirs(out_dir, exist_ok=True)

    # Clear output files
    with open(asm_file, "w") as f:
        f.write("\n")
    with open(header_file, "w") as f:
        f.write("")

    print("Generating filesystem...")

    processed_files = set()

    # Process priority file first
    if priority_file:
        first_file_path = os.path.abspath(os.path.join(src_dir, priority_file))
        if os.path.isfile(first_file_path):
            process_file(first_file_path, asm_file, header_file)
            processed_files.add(first_file_path)
        else:
            print(f"Warning: priority file '{priority_file}' not found in input folder.")

    # Process remaining files
    for root, dirs, files in os.walk(src_dir):
        for file in files:
            file_path = os.path.abspath(os.path.join(root, file))
            if file_path in processed_files:
                continue
            ext = file.split(".")[-1].lower()
            if ext not in extensions:
                continue
            process_file(file_path, asm_file, header_file)
            processed_files.add(file_path)

if __name__ == "__main__":
    main()
