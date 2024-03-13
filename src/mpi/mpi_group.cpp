#include <mpi.h>

#include "mpi_interposition.h"

extern "C" {

/***********************************************
 * MPI group
 ***********************************************/

#if MMCSO_OFFLOAD_NOT_IMPLEMENTED_YET

int       MPI_Group_compare(MPI_Group group1, MPI_Group group2, int *result);
int       MPI_Group_difference(MPI_Group group1, MPI_Group group2, MPI_Group *newgroup);
int       MPI_Group_excl(MPI_Group group, int n, const int ranks[], MPI_Group *newgroup);

int       MPI_Group_free(MPI_Group *group);
int       MPI_Group_incl(MPI_Group group, int n, const int ranks[], MPI_Group *newgroup);
int       MPI_Group_intersection(MPI_Group group1, MPI_Group group2, MPI_Group *newgroup);
int       MPI_Group_range_excl(MPI_Group group, int n, int ranges[][3], MPI_Group *newgroup);
int       MPI_Group_range_incl(MPI_Group group, int n, int ranges[][3], MPI_Group *newgroup);
int       MPI_Group_rank(MPI_Group group, int *rank);
int       MPI_Group_size(MPI_Group group, int *size);
int       MPI_Group_translate_ranks(MPI_Group group1, int n, const int ranks1[], MPI_Group group2, int ranks2[]);
int       MPI_Group_union(MPI_Group group1, MPI_Group group2, MPI_Group *newgroup);

#endif /* MMCSO_OFFLOAD_NOT_IMPLEMENTED_YET */

} /* extern "C" */
