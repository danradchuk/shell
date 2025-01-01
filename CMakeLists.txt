cmake_minimum_required(VERSION 3.15)
project(my_c_project LANGUAGES C)

# Set the C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Export compile_commands.json for LSP
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set the compiler to Clang
set(CMAKE_C_COMPILER clang)

# Build types
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Source files
file(GLOB_RECURSE SOURCES src/*.c)
include_directories(include)

# Create the executable
add_executable(my_c_project ${SOURCES})

# Add compile options
target_compile_options(my_c_project PRIVATE -Wall -Wextra -pedantic)

# Unit tests (optional)
# enable_testing()
# add_subdirectory(tests)
