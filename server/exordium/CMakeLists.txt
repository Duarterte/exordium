cmake_minimum_required(VERSION 3.10)

project(exordium)

# Set the C and C++ compilers
set(CMAKE_C_COMPILER "g++")
set(CMAKE_CXX_COMPILER "g++")

# Set compiler flags
set(CMAKE_CXX_FLAGS "-std=c++23 -Wall -Wextra")

# Set linker flags (if any)
# set(CMAKE_EXE_LINKER_FLAGS "")

# Define source files
file(GLOB SOURCE_FILES src/*.cpp)


# Include directories
include_directories(${CMAKE_SOURCE_DIR}/includes)

# Define the executable
add_executable(exordium ${SOURCE_FILES})

# Link against sodium
target_link_libraries(exordium sodium ssl crypto)

# Set the output directory for the executable to the 'bin' folder at the project root
set_target_properties(exordium PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)

# Custom target to remove CMake-related build artifacts
add_custom_target(clean_cmake
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_BINARY_DIR}"
    COMMAND ${CMAKE_COMMAND} -E remove "${CMAKE_SOURCE_DIR}/CMakeCache.txt"   # Remove CMakeCache.txt at source root
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMENT "Remove CMake build directory and cache"
)