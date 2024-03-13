#include <mpi.h>

#include "mpi_interposition.h"

extern "C" {

/***********************************************
 * MPI info
 ***********************************************/

#if MMCSO_OFFLOAD_NOT_IMPLEMENTED_YET

int      MPI_Info_create(MPI_Info *info);
int      MPI_Info_delete(MPI_Info info, const char *key);
int      MPI_Info_dup(MPI_Info info, MPI_Info *newinfo);

int      MPI_Info_free(MPI_Info *info);
int      MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);
int      MPI_Info_get_nkeys(MPI_Info info, int *nkeys);
int      MPI_Info_get_nthkey(MPI_Info info, int n, char *key);
int      MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);
int      MPI_Info_set(MPI_Info info, const char *key, const char *value);

#endif /* MMCSO_OFFLOAD_NOT_IMPLEMENTED_YET */

} /* extern "C" */
