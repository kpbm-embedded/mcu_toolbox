# Host unit tests for lib/ (plain C asserts, no framework): cmake -S tests -B b && cmake --build b && ctest --test-dir b
cmake_minimum_required(VERSION 3.20)
project({{NAME}}_host_tests C)
enable_testing()
add_executable(test_ringbuf test_ringbuf.c ../lib/ringbuf.c)
target_include_directories(test_ringbuf PRIVATE ../lib)
target_compile_options(test_ringbuf PRIVATE -Wall -Wextra -Werror)
add_test(NAME ringbuf COMMAND test_ringbuf)
