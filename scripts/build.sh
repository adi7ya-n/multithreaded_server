#!/bin/bash

cd $(git rev-parse --show-toplevel)

# Delete build folder if it already exists
rm -rf build/

# Create a build directory in root
mkdir build/
cd build
cmake ../