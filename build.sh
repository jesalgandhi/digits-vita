#!/bin/bash
#
# rebuild.sh
#
# This script cleans the current directory of CMake and Vita build artifacts
# by removing files with common extensions and generated directories, then
# re-runs cmake and make.
#
# Usage:
#   chmod +x rebuild.sh
#   ./rebuild.sh

echo "Cleaning build artifacts..."

# Remove CMake-generated files and directories
rm -f CMakeCache.txt cmake_install.cmake Makefile
rm -rf CMakeFiles

# Remove generated Vita build artifacts by common extensions:
#   *.self and *.self.out (the SELF binaries)
#   *.velf (the Sony ELF file)
#   *.vpk, *.vpk.out, and *.vpk_param.sfo (the packaged files)
rm -f *.self *.self.out *.velf *.vpk *.vpk.out *.vpk_param.sfo

echo "Clean complete."

echo "Running cmake ."
cmake .

echo "Building project using all processors..."
make -j$(nproc)

echo "Build complete."

