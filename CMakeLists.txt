cmake_minimum_required(VERSION 4.1)
project(my_custom_memory_allocator C)

include(CTest)

set(CMAKE_C_STANDARD 11)
set(ENV{LSAN_OPTIONS} "verbosity=1:log_threads=1")
set(ENV{ASAN_OPTIONS} "detect_leaks=0;abort_on_error=1")

set(LIB_FILES allocator.c allocator.h)

add_library(allocator_lib ${LIB_FILES})

target_compile_options(allocator_lib PUBLIC
        -fno-omit-frame-pointer
        -fsanitize=address
        -g
        )

target_link_options(allocator_lib PUBLIC
        -fno-omit-frame-pointer
        -fsanitize=address
        )

file(GLOB test_files "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.c")

foreach(file IN LISTS test_files)
        get_filename_component(test_name ${file} NAME_WE)
        message(${file})
        add_executable(${test_name} ${file})
        target_link_libraries(${test_name} PUBLIC allocator_lib)
        add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()