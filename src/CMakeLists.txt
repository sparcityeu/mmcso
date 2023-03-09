set(
    SRC
    mpi_interposition.cpp
    util.cpp
)

add_library(mmcso SHARED ${SRC})

target_include_directories(mmcso PRIVATE ${PROJECT_SOURCE_DIR}/external/atomic_queue/include)
target_link_libraries(mmcso PRIVATE MPI::MPI_CXX)
