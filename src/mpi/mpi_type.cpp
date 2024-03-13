#include <mpi.h>

#include "mpi_interposition.h"

extern "C" {

/***********************************************
 * MPI types
 ***********************************************/

#if MMCSO_OFFLOAD_NOT_IMPLEMENTED_YET

int      MPI_Type_commit(MPI_Datatype *type);
int      MPI_Type_contiguous(int count, MPI_Datatype oldtype, MPI_Datatype *newtype);
int      MPI_Type_create_darray(int           size,
                                int           rank,
                                int           ndims,
                                const int     gsize_array[],
                                const int     distrib_array[],
                                const int     darg_array[],
                                const int     psize_array[],
                                int           order,
                                MPI_Datatype  oldtype,
                                MPI_Datatype *newtype);
int      MPI_Type_create_f90_complex(int p, int r, MPI_Datatype *newtype);
int      MPI_Type_create_f90_integer(int r, MPI_Datatype *newtype);
int      MPI_Type_create_f90_real(int p, int r, MPI_Datatype *newtype);
int      MPI_Type_create_hindexed_block(
         int count, int blocklength, const MPI_Aint array_of_displacements[], MPI_Datatype oldtype, MPI_Datatype *newtype);
int MPI_Type_create_hindexed(int            count,
                             const int      array_of_blocklengths[],
                             const MPI_Aint array_of_displacements[],
                             MPI_Datatype   oldtype,
                             MPI_Datatype  *newtype);
int MPI_Type_create_hvector(int count, int blocklength, MPI_Aint stride, MPI_Datatype oldtype, MPI_Datatype *newtype);
int MPI_Type_create_keyval(MPI_Type_copy_attr_function   *type_copy_attr_fn,
                           MPI_Type_delete_attr_function *type_delete_attr_fn,
                           int                           *type_keyval,
                           void                          *extra_state);
int MPI_Type_create_indexed_block(
    int count, int blocklength, const int array_of_displacements[], MPI_Datatype oldtype, MPI_Datatype *newtype);
int          MPI_Type_create_struct(int                count,
                                    const int          array_of_block_lengths[],
                                    const MPI_Aint     array_of_displacements[],
                                    const MPI_Datatype array_of_types[],
                                    MPI_Datatype      *newtype);
int          MPI_Type_create_subarray(int           ndims,
                                      const int     size_array[],
                                      const int     subsize_array[],
                                      const int     start_array[],
                                      int           order,
                                      MPI_Datatype  oldtype,
                                      MPI_Datatype *newtype);
int          MPI_Type_create_resized(MPI_Datatype oldtype, MPI_Aint lb, MPI_Aint extent, MPI_Datatype *newtype);
int          MPI_Type_delete_attr(MPI_Datatype type, int type_keyval);
int          MPI_Type_dup(MPI_Datatype type, MPI_Datatype *newtype);
int          MPI_Type_free(MPI_Datatype *type);
int          MPI_Type_free_keyval(int *type_keyval);

int          MPI_Type_get_attr(MPI_Datatype type, int type_keyval, void *attribute_val, int *flag);
int          MPI_Type_get_contents(MPI_Datatype mtype,
                                   int          max_integers,
                                   int          max_addresses,
                                   int          max_datatypes,
                                   int          array_of_integers[],
                                   MPI_Aint     array_of_addresses[],
                                   MPI_Datatype array_of_datatypes[]);
int MPI_Type_get_envelope(MPI_Datatype type, int *num_integers, int *num_addresses, int *num_datatypes, int *combiner);
int MPI_Type_get_extent(MPI_Datatype type, MPI_Aint *lb, MPI_Aint *extent);
int MPI_Type_get_extent_x(MPI_Datatype type, MPI_Count *lb, MPI_Count *extent);
int MPI_Type_get_name(MPI_Datatype type, char *type_name, int *resultlen);
int MPI_Type_get_true_extent(MPI_Datatype datatype, MPI_Aint *true_lb, MPI_Aint *true_extent);
int MPI_Type_get_true_extent_x(MPI_Datatype datatype, MPI_Count *true_lb, MPI_Count *true_extent);
int MPI_Type_indexed(int           count,
                     const int     array_of_blocklengths[],
                     const int     array_of_displacements[],
                     MPI_Datatype  oldtype,
                     MPI_Datatype *newtype);
int MPI_Type_match_size(int typeclass, int size, MPI_Datatype *type);
int MPI_Type_set_attr(MPI_Datatype type, int type_keyval, void *attr_val);
int MPI_Type_set_name(MPI_Datatype type, const char *type_name);
int MPI_Type_size(MPI_Datatype type, int *size);
int MPI_Type_size_x(MPI_Datatype type, MPI_Count *size);
int MPI_Type_vector(int count, int blocklength, int stride, MPI_Datatype oldtype, MPI_Datatype *newtype);

#endif /* MMCSO_OFFLOAD_NOT_IMPLEMENTED_YET */

} /* extern "C" */
