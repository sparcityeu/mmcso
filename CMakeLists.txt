# Project structure: The Pitchfork Layout (PFL)

cmake_minimum_required(VERSION 3.12)

project(
    mmcso
    VERSION 0.2
    LANGUAGES CXX
    DESCRIPTION "Multithreaded MPI software communication offloading library."
    )

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

include(ClangTidy)

# Set interprocedural (link-time) optimization
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)

if(ipo_supported)
    message(STATUS "IPO / LTO enabled")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    # set_property(TARGET example PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(STATUS "IPO / LTO not supported: <${ipo_error}>")
endif()

# Set warning flags
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # using Clang
    add_compile_options(-Wall -Wextra -Wpedantic) # -Wconversion
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # using GCC
    add_compile_options(-Wall -Wextra -Wpedantic) # -Wconversion
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    # using Intel C++
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # using Visual Studio C++
endif()

# Set optimization flags
# add_compile_options(-march=native)
string(APPEND CMAKE_CXX_FLAGS_RELEASE " -O3 -march=native -mtune=native ")
# add_compile_options(
# "$<$<COMPILE_LANGUAGE:CXX>:-Ofast;-DNDEBUG;-std=c++20;-march=native;-fpic;-ftree-vectorize>"
# )

# add_compile_options(-fsanitize=address)
# add_link_options(-fsanitize=address)

# add_compile_options(-fsanitize=thread)
# add_link_options(-fsanitize=thread)

find_package(MPI REQUIRED)
message(STATUS "Run: ${MPIEXEC} ${MPIEXEC_NUMPROC_FLAG} ${MPIEXEC_MAX_NUMPROCS} ${MPIEXEC_PREFLAGS} EXECUTABLE ${MPIEXEC_POSTFLAGS} ARGS")

include_directories(SYSTEM ${MPI_INCLUDE_PATH})

execute_process(COMMAND getconf LEVEL1_DCACHE_LINESIZE OUTPUT_VARIABLE CLSIZE OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "System cache line size : ${CLSIZE}")
add_compile_definitions(CLSIZE=${CLSIZE})

add_subdirectory(src)
add_subdirectory(bench)
