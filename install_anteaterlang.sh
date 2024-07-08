#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

# Navigate to the src directory
cd src

# Clean up any previously created files
make clean

# Build the project
make

# Install the static library and executables
sudo make install

# Clean up
make clean

echo "AnteaterLang has been successfully installed."
