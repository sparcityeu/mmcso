set(
    SRC
    mpi/mpi_c2f2c.cpp
    mpi/mpi_collective.cpp
    mpi/mpi_comm.cpp
    mpi/mpi_file.cpp
    mpi/mpi_group.cpp
    mpi/mpi_info.cpp
    mpi/mpi_interposition.cpp
    mpi/mpi_one_sided.cpp
    mpi/mpi_p2p.cpp
    mpi/mpi_partitioned.cpp
    mpi/mpi_request_status.cpp
    mpi/mpi_topo.cpp
    mpi/mpi_type.cpp
    mpi/mpi_win.cpp
    # mpix_partitioned_collective.cpp
    util.cpp
)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

add_library(mmcso SHARED ${SRC})

target_include_directories(mmcso PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(mmcso PRIVATE ${PROJECT_SOURCE_DIR}/external/atomic_queue/include)
target_include_directories(mmcso PRIVATE ${PROJECT_SOURCE_DIR}/external/)

target_link_libraries(mmcso PRIVATE MPI::MPI_CXX)
