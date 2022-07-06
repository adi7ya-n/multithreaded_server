#!/bin/bash

# Go to the top level
cd $(git rev-parse --show-toplevel)

# C++ formatting
clang-format --style=file -i src/engine/game/Game.cpp \
                             src/engine/game/include/Game.hpp \
                             src/engine/include/Server.hpp \
                             src/engine/Server.cpp \
                             src/logger/include/Logger.hpp \
                             src/logger/Logger.cpp \
                             src/main.cpp

# Python formatting
autopep8 -r --in-place client/
