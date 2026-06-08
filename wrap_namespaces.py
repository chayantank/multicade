import os
import glob
import re

def process_file(filepath, ns_name):
    with open(filepath, 'r') as f:
        lines = f.readlines()
    
    out_lines = []
    in_namespace = False
    
    # We want to keep includes, #pragma once, and the outermost #ifndef / #define include guards outside the namespace.
    # We will find the last include line.
    
    last_include_idx = -1
    for i, line in enumerate(lines):
        if line.startswith('#include'):
            last_include_idx = i
            
    # Also find if there are include guards at the very top
    guard_define_idx = -1
    if len(lines) > 1 and lines[0].startswith('#ifndef'):
        if lines[1].startswith('#define'):
            guard_define_idx = 1
            
    start_ns_idx = max(last_include_idx, guard_define_idx) + 1
    
    # Check if there's an #endif at the very end for the guard
    end_ns_idx = len(lines)
    if guard_define_idx != -1:
        for i in range(len(lines)-1, -1, -1):
            if lines[i].startswith('#endif'):
                end_ns_idx = i
                break
                
    with open(filepath, 'w') as f:
        for i in range(start_ns_idx):
            f.write(lines[i])
        
        f.write(f"\nnamespace {ns_name} {{\n\n")
        
        for i in range(start_ns_idx, end_ns_idx):
            f.write(lines[i])
            
        f.write(f"\n}} // namespace {ns_name}\n")
        
        for i in range(end_ns_idx, len(lines)):
            f.write(lines[i])

def main():
    doom_files = glob.glob('/Users/chayan/Documents/personal_projects/esp32/multicade/src/doom/*.h') + \
                 glob.glob('/Users/chayan/Documents/personal_projects/esp32/multicade/src/doom/*.cpp') + \
                 glob.glob('/Users/chayan/Documents/personal_projects/esp32/multicade/src/doom/*.ino')
                 
    for f in doom_files:
        print("Processing", f)
        process_file(f, "Doom")

    tetris_files = glob.glob('/Users/chayan/Documents/personal_projects/esp32/multicade/src/tetris/*.h') + \
                   glob.glob('/Users/chayan/Documents/personal_projects/esp32/multicade/src/tetris/*.cpp')
                   
    for f in tetris_files:
        print("Processing", f)
        process_file(f, "Tetris")

if __name__ == '__main__':
    main()
