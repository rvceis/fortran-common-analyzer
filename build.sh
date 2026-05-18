#!/bin/bash

# Exit on any error
set -e

mkdir -p build
cd build

# Configure the project with CMake
cmake ..

# Build the project using multiple cores
make -j$(nproc)

echo "Build completed successfully. Executable is located at build/analyzer"