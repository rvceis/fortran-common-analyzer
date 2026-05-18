cmake_minimum_required(VERSION 3.13.4)
project(FortranCommonAnalyzer)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find LLVM 17
find_package(LLVM 17 REQUIRED CONFIG)

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

# Add LLVM definitions and include directories
add_definitions(${LLVM_DEFINITIONS})
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

# Create executable
add_executable(analyzer
    src/main.cpp
    src/flang_parser.cpp
)

# Find all LLVM libraries to match the behavior of `llvm-config --libs`
llvm_map_components_to_libnames(llvm_libs support core)

# Link the Flang and LLVM libraries
target_link_libraries(analyzer
    PRIVATE
    FortranParser
    FortranCommon
    ${llvm_libs}
)
