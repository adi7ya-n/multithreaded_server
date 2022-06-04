#!/bin/bash

# C++ formatting
clang-format --style=file game/Game.hpp server/main.cpp server/Server.hpp utils/logger/Logger.hpp utils/logger/Logger.cpp utils/network/Network.hpp utils/Utilities.hpp > /dev/null

# Python formatting
autopep8 -r --in-place tools/ client/