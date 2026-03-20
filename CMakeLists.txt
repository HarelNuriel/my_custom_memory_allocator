cmake_minimum_required(VERSION 4.1)
project(my_custom_memory_allocator C)

set(CMAKE_C_STANDARD 11)
set(ENV{LSAN_OPTIONS} "verbosity=1:log_threads=1")

set(SOURCE_FILES test.c
        allocator.c
        allocator.h)

add_executable(my_custom_memory_allocator ${SOURCE_FILES})

target_compile_options(my_custom_memory_allocator PRIVATE
        -fno-omit-frame-pointer
        -fsanitize=address
        -O1
        )

target_link_options(my_custom_memory_allocator PRIVATE
        -fno-omit-frame-pointer
        -fsanitize=address
        -O1
        )