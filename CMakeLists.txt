cmake_minimum_required(VERSION 4.1)
project(my_custom_memory_allocator C)

include(CTest)

set(CMAKE_C_STANDARD 11)
set(ENV{LSAN_OPTIONS} "verbosity=1:log_threads=1")
set(ENV{ASAN_OPTIONS} "detect_leaks=0;abort_on_error=1")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(WRAPPER_SCRIPT ${CMAKE_SOURCE_DIR}/test_wrapper.sh)
set(LIB_FILES arena.h allocator.c allocator.h)
set(EXPECTED_TO_FAIL double_free)

if(EXISTS "${WRAPPER_SCRIPT}" AND IS_EXECUTABLE "${WRAPPER_SCRIPT}")
    message(STATUS "${WRAPPER_SCRIPT} exists and executale.")
else()
    message(FATAL_ERROR "Check That ${WRAPPER_SCRIPT} exists and executale.")
endif()

add_library(allocator_lib ${LIB_FILES})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(allocator_lib PUBLIC
            -fno-omit-frame-pointer
            -fsanitize=address
            -g
            )

    target_link_options(allocator_lib PUBLIC
            -fno-omit-frame-pointer
            -fsanitize=address
            )
else()
    target_compile_options(allocator_lib PUBLIC
        -O3
    )
endif()

file(GLOB test_files "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.c")

foreach(file IN LISTS test_files)
    get_filename_component(test_name ${file} NAME_WE)
    add_executable(${test_name} ${file})
    target_link_libraries(${test_name} PUBLIC allocator_lib)
    if(${test_name} IN_LIST EXPECTED_TO_FAIL)
        add_test(NAME ${test_name} COMMAND
                sh -c "${WRAPPER_SCRIPT} $<TARGET_FILE:${test_name}>")
    else()
        add_test(NAME ${test_name} COMMAND ${test_name})
    endif()
endforeach()
