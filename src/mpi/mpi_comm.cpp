#include <mpi.h>

#include "mpi_interposition.h"

extern "C" {

/***********************************************
 * MPI communicator
 ***********************************************/

/** TODO: is offloading of these functions really required? */
#if OFFLOAD_MPI_COMM_RANK_SIZE
int MPI_Comm_rank(MPI_Comm comm, int *rank)
{
    MPI_Request request; // = MPI_REQUEST_NULL;
    oe.post(new mmcso::OffloadCommand{std::function{[=](MPI_Request *request_) {
                                          (void)request_;
                                          return PMPI_Comm_rank(comm, rank);
                                      }},
                                      &request,
                                      true});
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}

int MPI_Comm_size(MPI_Comm comm, int *size)
{
    MPI_Request request; // = MPI_REQUEST_NULL;
    oe.post(new mmcso::OffloadCommand{std::function{[=](MPI_Request *request_) {
                                          (void)request_;
                                          return PMPI_Comm_size(comm, size);
                                      }},
                                      &request,
                                      true});
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}
#endif

int MPI_Comm_idup(MPI_Comm comm, MPI_Comm *newcomm, MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{
        std::function{[=](MPI_Request *request_) { return PMPI_Comm_idup(comm, newcomm, request_); }}, request});
    return MPI_SUCCESS;
}

int MPI_Comm_dup(MPI_Comm comm, MPI_Comm *newcomm)
{
    MPI_Request request;
    MPI_Comm_idup(comm, newcomm, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}

#if MMCSO_OFFLOAD_NOT_IMPLEMENTED_YET

int            MPI_Comm_accept(const char *port_name, MPI_Info info, int root, MPI_Comm comm, MPI_Comm *newcomm);

int            MPI_Comm_call_errhandler(MPI_Comm comm, int errorcode);
int            MPI_Comm_compare(MPI_Comm comm1, MPI_Comm comm2, int *result);
int            MPI_Comm_connect(const char *port_name, MPI_Info info, int root, MPI_Comm comm, MPI_Comm *newcomm);
int            MPI_Comm_create_errhandler(MPI_Comm_errhandler_function *function, MPI_Errhandler *errhandler);
int            MPI_Comm_create_keyval(MPI_Comm_copy_attr_function   *comm_copy_attr_fn,
                                      MPI_Comm_delete_attr_function *comm_delete_attr_fn,
                                      int                           *comm_keyval,
                                      void                          *extra_state);
int            MPI_Comm_create_group(MPI_Comm comm, MPI_Group group, int tag, MPI_Comm *newcomm);
int            MPI_Comm_create(MPI_Comm comm, MPI_Group group, MPI_Comm *newcomm);
int            MPI_Comm_delete_attr(MPI_Comm comm, int comm_keyval);
int            MPI_Comm_disconnect(MPI_Comm *comm);
int            MPI_Comm_dup(MPI_Comm comm, MPI_Comm *newcomm);
int            MPI_Comm_idup(MPI_Comm comm, MPI_Comm *newcomm, MPI_Request *request);
int            MPI_Comm_dup_with_info(MPI_Comm comm, MPI_Info info, MPI_Comm *newcomm);

int            MPI_Comm_free_keyval(int *comm_keyval);
int            MPI_Comm_free(MPI_Comm *comm);
int            MPI_Comm_get_attr(MPI_Comm comm, int comm_keyval, void *attribute_val, int *flag);
int            MPI_Comm_get_errhandler(MPI_Comm comm, MPI_Errhandler *erhandler);
int            MPI_Comm_get_info(MPI_Comm comm, MPI_Info *info_used);
int            MPI_Comm_get_name(MPI_Comm comm, char *comm_name, int *resultlen);
int            MPI_Comm_get_parent(MPI_Comm *parent);
int            MPI_Comm_group(MPI_Comm comm, MPI_Group *group);
int            MPI_Comm_join(int fd, MPI_Comm *intercomm);
int            MPI_Comm_rank(MPI_Comm comm, int *rank);
int            MPI_Comm_remote_group(MPI_Comm comm, MPI_Group *group);
int            MPI_Comm_remote_size(MPI_Comm comm, int *size);
int            MPI_Comm_set_attr(MPI_Comm comm, int comm_keyval, void *attribute_val);
int            MPI_Comm_set_errhandler(MPI_Comm comm, MPI_Errhandler errhandler);
int            MPI_Comm_set_info(MPI_Comm comm, MPI_Info info);
int            MPI_Comm_set_name(MPI_Comm comm, const char *comm_name);
int            MPI_Comm_size(MPI_Comm comm, int *size);
int            MPI_Comm_spawn(const char *command,
                              char       *argv[],
                              int         maxprocs,
                              MPI_Info    info,
                              int         root,
                              MPI_Comm    comm,
                              MPI_Comm   *intercomm,
                              int         array_of_errcodes[]);
int            MPI_Comm_spawn_multiple(int            count,
                                       char          *array_of_commands[],
                                       char         **array_of_argv[],
                                       const int      array_of_maxprocs[],
                                       const MPI_Info array_of_info[],
                                       int            root,
                                       MPI_Comm       comm,
                                       MPI_Comm      *intercomm,
                                       int            array_of_errcodes[]);
int            MPI_Comm_split(MPI_Comm comm, int color, int key, MPI_Comm *newcomm);
int            MPI_Comm_split_type(MPI_Comm comm, int split_type, int key, MPI_Info info, MPI_Comm *newcomm);
int            MPI_Comm_test_inter(MPI_Comm comm, int *flag);

#endif /* MMCSO_OFFLOAD_NOT_IMPLEMENTED_YET */

} /* extern "C" */