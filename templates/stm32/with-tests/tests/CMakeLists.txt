# Host unit tests (Unity). Plain host CMake project, no Cube/HAL:
#   cmake -G Ninja -S tests -B build-host && cmake --build build-host && ctest --test-dir build-host --output-on-failure
cmake_minimum_required(VERSION 3.20)
project({{NAME}}_tests C)
enable_testing()
add_library(unity unity/unity.c)
target_include_directories(unity PUBLIC unity)
add_executable(test_logic test_logic.c ../src/logic.c)
target_include_directories(test_logic PRIVATE ../src)
target_link_libraries(test_logic unity)
add_test(NAME logic COMMAND test_logic)
