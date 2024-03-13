#include <mpi.h>

#include "mpi_interposition.h"

extern "C" {

/***********************************************
 * MPI requests and statuses
 ***********************************************/

#if MMCSO_OFFLOAD_NOT_IMPLEMENTED_YET
int MPI_Cancel(MPI_Request *request);
int MPI_Get_count(const MPI_Status *status, MPI_Datatype datatype, int *count);
int MPI_Get_elements(const MPI_Status *status, MPI_Datatype datatype, int *count);
int MPI_Get_elements_x(const MPI_Status *status, MPI_Datatype datatype, MPI_Count *count);
int       MPI_Grequest_complete(MPI_Request request);
int       MPI_Grequest_start(MPI_Grequest_query_function  *query_fn,
                             MPI_Grequest_free_function   *free_fn,
                             MPI_Grequest_cancel_function *cancel_fn,
                             void                         *extra_state,
                             MPI_Request                  *request);

int         MPI_Request_free(MPI_Request *request);
int         MPI_Request_get_status(MPI_Request request, int *flag, MPI_Status *status);

int MPI_Status_set_cancelled(MPI_Status *status, int flag);
int MPI_Status_set_elements(MPI_Status *status, MPI_Datatype datatype, int count);
int MPI_Status_set_elements_x(MPI_Status *status, MPI_Datatype datatype, MPI_Count count);

#endif /* MMCSO_OFFLOAD_NOT_IMPLEMENTED_YET */

} /* extern "C" */
