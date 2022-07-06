add_library(game Game.cpp include/Game.hpp)
target_include_directories(game PUBLIC include/)
target_link_libraries(game PUBLIC logger)