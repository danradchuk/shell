cmake_minimum_required(VERSION 3.15)

project(shell LANGUAGES C)

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

# Create the executable
add_executable(${PROJECT_NAME} src/main.c src/utils.c src/job.c src/slice.c)

# Add compile options
target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address -Wall -Wextra -pedantic)
target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
target_include_directories(${PROJECT_NAME} PRIVATE include)

# Unit tests (optional)
# enable_testing()
# add_subdirectory(tests)
