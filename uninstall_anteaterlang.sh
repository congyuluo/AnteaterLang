#!/bin/bash

# Define installation paths
PREFIX_LIB=/usr/local/lib
PREFIX_BIN=/usr/local/bin
INCLUDE_DIR=/usr/local/include/anteaterlang

# Remove the compiled executable
if [ -f $PREFIX_BIN/antlang ]; then
    sudo rm -f $PREFIX_BIN/antlang
    echo "Removed $PREFIX_BIN/antlang"
else
    echo "$PREFIX_BIN/antlang not found"
fi

# Remove the antcompilec script
if [ -f $PREFIX_BIN/antcompilec ]; then
    sudo rm -f $PREFIX_BIN/antcompilec
    echo "Removed $PREFIX_BIN/antcompilec"
else
    echo "$PREFIX_BIN/antcompilec not found"
fi

# Remove the static library
if [ -f $PREFIX_LIB/liblang.a ]; then
    sudo rm -f $PREFIX_LIB/liblang.a
    echo "Removed $PREFIX_LIB/liblang.a"
else
    echo "$PREFIX_LIB/liblang.a not found"
fi

# Remove the header files
if [ -d $INCLUDE_DIR ]; then
    sudo rm -rf $INCLUDE_DIR
    echo "Removed $INCLUDE_DIR"
else
    echo "$INCLUDE_DIR not found"
fi

echo "Uninstallation complete."
