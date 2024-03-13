#include <mpi.h>

#include "mpi_interposition.h"

extern "C" {

/***********************************************
 * MPI partitioned P2P
 ***********************************************/

#if MMCSO_OFFLOAD_NOT_IMPLEMENTED_YET

int MPI_Bsend_init(
    const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request);
    
int MPI_Recv_init(
    void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Request *request);

int         MPI_Rsend_init(
            const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request);

int MPI_Send_init(
    const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request);

int MPI_Ssend_init(
    const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request);

int MPI_Start(MPI_Request *request);
int MPI_Startall(int count, MPI_Request array_of_requests[]);

#endif /* MMCSO_OFFLOAD_NOT_IMPLEMENTED_YET */

} /* extern "C" */
