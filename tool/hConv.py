import os
import re
import argparse

# Allowed extensions
ALLOWED_EXTENSIONS = {".zmap", ".zscene", ".bin", ".yaz0", ".tbl"}

def sanitize_name(filename):
    """Convert filename to a valid C identifier in uppercase."""
    name, ext = os.path.splitext(filename)
    full_name = f"{name}_{ext[1:]}" if ext else name
    full_name = re.sub(r'\W', '_', full_name)
    return full_name.upper()

def process_file(input_path, output_folder, array_name):
    """Convert and save each file as a C header with #ifndef guard."""
    guard_name = f"{array_name}_H"
    output_path = os.path.join(output_folder, f"{array_name}.h")

    # Use os.stat to get file size directly
    file_size = os.stat(input_path).st_size

    with open(input_path, "rb") as f:
        content = f.read()

    with open(output_path, "w") as f:
        f.write(f"#ifndef {guard_name}\n")
        f.write(f"#define {guard_name}\n\n")
        f.write(f"unsigned char {array_name}[] = {{\n")

        for i, byte in enumerate(content):
            f.write(f"0x{byte:02X}, ")
            if (i + 1) % 12 == 0:
                f.write("\n")

        f.write(f"\n}};\n")
        # f.write(f"unsigned int {array_name}_LEN = {file_size};\n\n")
        f.write(f"#endif // {guard_name}\n")

    print(f"Generated {output_path}")

def generate_collective_header(input_folder, output_folder, all_names, all_lengths):
    """Generate a collective header file with forward declarations."""
    collective_header_path = os.path.join(output_folder, f"{os.path.basename(input_folder)}.h")
    guard_name = f"{os.path.basename(input_folder).upper()}_FILES"
    
    with open(collective_header_path, "w") as f:
        f.write(f"#ifndef {guard_name}\n")
        f.write(f"#define {guard_name}\n\n")
        
        for name, length in zip(all_names, all_lengths):
            f.write(f"extern unsigned char {name}[];\n")
            f.write(f"#define {name}_LEN {length}\n\n")
        
        f.write(f"#endif // {guard_name}\n")

    print(f"Generated collective header {collective_header_path}")

def main(input_folder, output_folder):
    # Create the base output folder structure: src/fileHeaders/[first input folder name]
    base_output_folder = os.path.join(output_folder, 'fileHeaders', input_folder)
    os.makedirs(base_output_folder, exist_ok=True)

    # List to hold all sanitized names and their corresponding lengths
    all_names = []
    all_lengths = []

    # Walk through the directory recursively
    for root, dirs, files in os.walk(input_folder):
        for filename in files:
            _, ext = os.path.splitext(filename)
            if ext.lower() not in ALLOWED_EXTENSIONS:
                continue  # Skip files with disallowed extensions

            input_path = os.path.join(root, filename)

            # Create a relative path from the input folder, and sanitize the file name
            sanitized_name = sanitize_name(filename)

            # Add the sanitized name to the list of all names
            all_names.append(sanitized_name)

            # Use os.stat to get the file size
            file_size = os.stat(input_path).st_size
            all_lengths.append(file_size)

            # Calculate the subfolder path in the output based on the directory structure
            relative_path = os.path.relpath(root, input_folder)
            output_subfolder = os.path.join(base_output_folder, relative_path)
            os.makedirs(output_subfolder, exist_ok=True)

            # Process the file (generate individual headers with #ifndef guards)
            process_file(input_path, output_subfolder, sanitized_name)

    # After processing all files, generate the collective header with forward declarations
    generate_collective_header(input_folder, base_output_folder, all_names, all_lengths)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Convert files to C headers")
    parser.add_argument("input_folder", help="Folder containing input files")
    parser.add_argument("output_folder", help="Folder to save generated headers")
    args = parser.parse_args()

    main(args.input_folder, args.output_folder)
