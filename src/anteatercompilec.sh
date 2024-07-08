#!/bin/bash

# Check if the correct number of arguments is provided
if [ "$#" -ne 1 ]; then
  echo "Usage: anteatercompilec <source_file.c>"
  exit 1
fi

# Set variables
SOURCE_FILE=$1
INCLUDE_DIR=/usr/local/include/anteaterlang
LIB_DIR=/usr/local/lib

# Compile the user function
gcc -fPIC -I"$INCLUDE_DIR" -c "$SOURCE_FILE" -o userFunctions.o

# Create the shared library
gcc -shared -o libuserFunctions.so userFunctions.o -L"$LIB_DIR" -llang

# Clean up
rm -f userFunctions.o

echo "Compilation successful. Shared library created: libuserFunctions.so"
