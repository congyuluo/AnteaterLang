#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

# Define the source and target paths
SRC_DIR=ide_src
IDE_SCRIPT=main.py
TARGET_SCRIPT=antide

# Get the current directory
CURRENT_DIR=$(pwd)

# Create the target script in /usr/local/bin
sudo bash -c "echo '#!/bin/bash' > /usr/local/bin/${TARGET_SCRIPT}"
sudo bash -c "echo 'python3 ${CURRENT_DIR}/${SRC_DIR}/${IDE_SCRIPT}' >> /usr/local/bin/${TARGET_SCRIPT}"

# Make the target script executable
sudo chmod +x /usr/local/bin/${TARGET_SCRIPT}

echo "Anteater IDE has been successfully installed as 'antide'."
