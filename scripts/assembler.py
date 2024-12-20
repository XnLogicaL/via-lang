import subprocess
import sys

def assemble_and_extract_machine_code(asm_file, asm_format):
    # Step 1: Use NASM to assemble the code
    # Assembly command: nasm -f elf32 -o example.o example.asm
    obj_file = asm_file.replace('.asm', '.o')

    try:
        subprocess.run(['nasm', '-f', asm_format, '-o', obj_file, asm_file], check=True)
    except subprocess.CalledProcessError as e:
        return []

    # Step 2: Use objdump to extract machine code (hex format) from the object file
    # objdump command: objdump -d example.o
    result = subprocess.run(['objdump', '-d', obj_file], capture_output=True, text=True)

    # Step 3: Parse the output and extract raw machine code
    machine_code = []
    for line in result.stdout.splitlines():
        if ' ' in line:
            parts = line.split()
            if len(parts) > 1 and all(c in '0123456789abcdef' for c in parts[0].lower()):
                machine_code.extend(parts[1:])

    # Convert the list of machine code bytes into a hex string
    return machine_code

if __name__ == '__main__':
    asm_file = input("Assembly file path: ")
    asm_format = input("Assembly format: ")
    raw_machine_code = assemble_and_extract_machine_code(asm_file, asm_format)
    print(f"Raw Machine Code: {raw_machine_code}")
