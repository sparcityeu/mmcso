add_executable(
    mt_overlap
    mt_overlap.cpp
    )

find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND)
    target_link_libraries(mt_overlap PUBLIC OpenMP::OpenMP_CXX)
endif()

execute_process(COMMAND getconf PAGE_SIZE OUTPUT_VARIABLE PAGESIZE OUTPUT_STRIP_TRAILING_WHITESPACE)
# message(STATUS "PAGESIZE:${PAGESIZE}")
target_compile_definitions(mt_overlap PRIVATE PAGESIZE=${PAGESIZE})

target_link_libraries(mt_overlap PUBLIC MPI::MPI_CXX)