# Host unit tests (Unity). Plain host CMake project, no Pico SDK:
#   cmake -G Ninja -S tests -B $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/<project>/host-tests && cmake --build ... && ctest --test-dir ...
cmake_minimum_required(VERSION 3.13)
project({{NAME}}_tests C)
enable_testing()
add_library(unity unity/unity.c)
target_include_directories(unity PUBLIC unity)
add_executable(test_logic test_logic.c ../src/logic.c)
target_include_directories(test_logic PRIVATE ../src)
target_link_libraries(test_logic unity)
add_test(NAME logic COMMAND test_logic)
