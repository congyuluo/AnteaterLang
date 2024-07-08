#!/bin/bash

# Define the target path
TARGET_SCRIPT=/usr/local/bin/antide

# Remove the Anteater IDE script
if [ -f $TARGET_SCRIPT ]; then
    sudo rm -f $TARGET_SCRIPT
    echo "Removed $TARGET_SCRIPT"
else
    echo "$TARGET_SCRIPT not found"
fi

echo "Anteater IDE has been successfully uninstalled."

