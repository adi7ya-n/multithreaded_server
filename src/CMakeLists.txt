add_subdirectory(logger)
add_subdirectory(engine)
add_executable(${PROJECT_NAME} main.cpp)

find_package(Boost REQUIRED COMPONENTS thread)
find_package(Boost REQUIRED COMPONENTS log)

target_link_libraries(${PROJECT_NAME} PUBLIC logger)
target_link_libraries(${PROJECT_NAME} PUBLIC server)
target_link_libraries(${PROJECT_NAME} PUBLIC Boost::thread Boost::log)