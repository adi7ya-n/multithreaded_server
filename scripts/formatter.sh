#!/bin/bash

# C++ formatting
clang-format --style=file src/server/include/Game.hpp src/server/main.cpp src/server/include/Server.hpp src/server/include/Logger.hpp src/server/include/Network.hpp src/server/include/Utilities.hpp > /dev/null

# Python formatting
autopep8 -r --in-place tools/ src/client/