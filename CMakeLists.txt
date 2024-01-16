# Project structure: The Pitchfork Layout (PFL)

cmake_minimum_required(VERSION 3.12)

project(
    mmcso
    VERSION 0.1
    LANGUAGES CXX
    )
    
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_CLANG_TIDY
  clang-tidy;
  -extra-arg=-std=c++17;
  -format-style='file';
  -header-filter=${CMAKE_CURRENT_SOURCE_DIR};
)

# add_compile_options(-march=native)
string(APPEND CMAKE_CXX_FLAGS_RELEASE " -march=native -mtune=native ")

find_package(MPI REQUIRED)
message(STATUS "Run: ${MPIEXEC} ${MPIEXEC_NUMPROC_FLAG} ${MPIEXEC_MAX_NUMPROCS} ${MPIEXEC_PREFLAGS} EXECUTABLE ${MPIEXEC_POSTFLAGS} ARGS")

include_directories(SYSTEM ${MPI_INCLUDE_PATH})

execute_process(COMMAND getconf LEVEL1_DCACHE_LINESIZE OUTPUT_VARIABLE CLSIZE OUTPUT_STRIP_TRAILING_WHITESPACE)
# message(STATUS "CLSIZE:${CLSIZE}")
add_compile_definitions(CLSIZE=${CLSIZE})

add_subdirectory(src)
add_subdirectory(bench)
