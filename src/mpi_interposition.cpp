#include <functional>
#include <future>

#include <mpi.h>

#include "array_request_manager.h"
// #include "hash_request_manager.h"

#include "atomic_queue.h"
#include "locked_queue.h"

#include "offload.h"

#define QUEUE_SIZE   1024
// #define REQUEST_SIZE 131072
#define REQUEST_SIZE 1024

using OE = mmcso::OffloadEngine<mmcso::AtomicQueue<QUEUE_SIZE>, mmcso::ArrayRequestManager<REQUEST_SIZE>>;
// using OE = mmcso::OffloadEngine<mmcso::LockedQueue, mmcso::ArrayRequestManager<REQUEST_SIZE>>;
static OE &oe = OE::instance();

extern "C" {

int MPI_Init_thread(int *argc, char ***argv, int required, int *provided)
{
    // future required because offload engine initializes MPI
    std::promise<int> provided_promise;
    std::future<int>  provided_future = provided_promise.get_future();

    oe.start(argc, argv, std::move(provided_promise));

    // MPI is guaranteed initialized (eventually by offloading thread) after get()
    *provided = provided_future.get();

    if (*provided == MPI_THREAD_SINGLE) {
        MPI_Abort(MPI_COMM_WORLD, 0);
    }

    if ((*provided == MPI_THREAD_SERIALIZED || *provided == MPI_THREAD_FUNNELED) && oe.num_threads == 1) {
        // using one offloading thread behaves as-if provided was MPI_THREAD_MULTIPLE
        *provided = MPI_THREAD_MULTIPLE;
    }

    return MPI_SUCCESS;
}

int MPI_Init(int *argc, char ***argv)
{
    /** TODO: use offloading when MPI_Init is called ? */
    int provided;
    return MPI_Init_thread(argc, argv, MPI_THREAD_SERIALIZED, &provided);
}

int MPI_Finalize()
{
    oe.stop(); /** TODO: fix race condition, according to thread sanitizer */
    PMPI_Finalize();
    return MPI_SUCCESS;
}

#if MMCSO_OFFLOAD_NOT_IMPLEMENTED_YET
int MPI_Waitany(int count, MPI_Request array_of_requests[], int *index, MPI_Status *status);
int MPI_Waitsome(int         incount,
                 MPI_Request array_of_requests[],
                 int        *outcount,
                 int         array_of_indices[],
                 MPI_Status  array_of_statuses[]);
#endif

int MPI_Wait(MPI_Request *request, MPI_Status *status)
{
    /** TODO: handle this case:
     * One is allowed to call MPI_Wait with a null or inactive request argument.
     * In this case the operation returns immediately with empty status.
     *
    if(*request == MPI_REQUEST_NULL) {
        // set empty status
        *status = ?
        return MPI_SUCCESS;
    }
    */

    oe.wait(request, status);
    return MPI_SUCCESS;
}

int MPI_Waitall(int count, MPI_Request array_of_requests[], MPI_Status *array_of_statuses)
{
    for (int i = 0; i < count; ++i) {
        MPI_Status *status = (array_of_statuses == MPI_STATUSES_IGNORE) ? MPI_STATUS_IGNORE : &array_of_statuses[i];
        MPI_Wait(&array_of_requests[i], status);
        /* spin */
    }
    return MPI_SUCCESS;
}

int MPI_Test(MPI_Request *request, int *flag, MPI_Status *status)
{
    /** TODO: handle this case:
     * One is allowed to call MPI_Test with a null or inactive request argument.
     * In such a case the operation returns with flag = true and empty status.
     */
    *flag = oe.test(request, status);
    return MPI_SUCCESS;
}


#if MMCSO_OFFLOAD_NOT_IMPLEMENTED_YET
int      MPI_Testall(int count, MPI_Request array_of_requests[], int *flag, MPI_Status array_of_statuses[]);
int      MPI_Testany(int count, MPI_Request array_of_requests[], int *index, int *flag, MPI_Status *status);
int      MPI_Test_cancelled(const MPI_Status *status, int *flag);
int      MPI_Testsome(int         incount,
                      MPI_Request array_of_requests[],
                      int        *outcount,
                      int         array_of_indices[],
                      MPI_Status  array_of_statuses[]);
#endif

int MPI_Irecv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{std::function{[=](MPI_Request *request_) {
                                          return PMPI_Irecv(buf, count, datatype, source, tag, comm, request_);
                                      }},
                                      request});
    return MPI_SUCCESS;
}

int MPI_Isend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{std::function{[=](MPI_Request *request_) {
                                          return PMPI_Isend(buf, count, datatype, dest, tag, comm, request_);
                                      }},
                                      request});
    return MPI_SUCCESS;
}

int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
{
    MPI_Request request;
    MPI_Isend(buf, count, datatype, dest, tag, comm, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}

int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status)
{
    MPI_Request request;
    MPI_Irecv(buf, count, datatype, source, tag, comm, &request);
    MPI_Wait(&request, status);
    return MPI_SUCCESS;
}

int MPI_Ibarrier(MPI_Comm comm, MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{
        std::function{[=](MPI_Request *request_) { return PMPI_Ibarrier(comm, request_); }}, request});
    return MPI_SUCCESS;
}

int MPI_Barrier(MPI_Comm comm)
{
    MPI_Request request;
    MPI_Ibarrier(comm, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}

int MPI_Ibcast(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm, MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{std::function{[=](MPI_Request *request_) {
                                          return PMPI_Ibcast(buffer, count, datatype, root, comm, request_);
                                      }},
                                      request});
    return MPI_SUCCESS;
}

int MPI_Bcast(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm)
{
    MPI_Request request;
    MPI_Ibcast(buffer, count, datatype, root, comm, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}

int MPI_Ireduce(const void  *sendbuf,
                void        *recvbuf,
                int          count,
                MPI_Datatype datatype,
                MPI_Op       op,
                int          root,
                MPI_Comm     comm,
                MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{std::function{[=](MPI_Request *request_) {
                                          return PMPI_Ireduce(
                                              sendbuf, recvbuf, count, datatype, op, root, comm, request_);
                                      }},
                                      request});
    return MPI_SUCCESS;
}

int MPI_Reduce(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm)
{
    MPI_Request request;
    MPI_Ireduce(sendbuf, recvbuf, count, datatype, op, root, comm, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}

int MPI_Iallgather(const void  *sendbuf,
                   int          sendcount,
                   MPI_Datatype sendtype,
                   void        *recvbuf,
                   int          recvcount,
                   MPI_Datatype recvtype,
                   MPI_Comm     comm,
                   MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{
        std::function{[=](MPI_Request *request_) {
            return PMPI_Iallgather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, request_);
        }},
        request});
    return MPI_SUCCESS;
}

int MPI_Allgather(const void  *sendbuf,
                  int          sendcount,
                  MPI_Datatype sendtype,
                  void        *recvbuf,
                  int          recvcount,
                  MPI_Datatype recvtype,
                  MPI_Comm     comm)
{
    MPI_Request request;
    MPI_Iallgather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}

int MPI_Iallreduce(const void  *sendbuf,
                   void        *recvbuf,
                   int          count,
                   MPI_Datatype datatype,
                   MPI_Op       op,
                   MPI_Comm     comm,
                   MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{std::function{[=](MPI_Request *request_) {
                                          return PMPI_Iallreduce(sendbuf, recvbuf, count, datatype, op, comm, request_);
                                      }},
                                      request});
    return MPI_SUCCESS;
}

int MPI_Allreduce(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm)
{
    MPI_Request request;
    MPI_Iallreduce(sendbuf, recvbuf, count, datatype, op, comm, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}

// TODO: is offloading of these functions really required?
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

#if REQUIRED
int MPI_Query_thread(int *provided);
int MPI_Comm_split(MPI_Comm comm, int color, int key, MPI_Comm *newcomm);
#endif

#if 1
int MPI_Win_create(void *base, MPI_Aint size, int disp_unit, MPI_Info info, MPI_Comm comm, MPI_Win *win)
{
    MPI_Request request; // = MPI_REQUEST_NULL;
    oe.post(new mmcso::OffloadCommand{
        std::function{[=](MPI_Request *request_) { return PMPI_Win_create(base, size, disp_unit, info, comm, win); }},
        &request,
        true});
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}

int MPI_Win_fence(int assert, MPI_Win win)
{
    MPI_Request request; // = MPI_REQUEST_NULL;
    oe.post(new mmcso::OffloadCommand{
        std::function{[=](MPI_Request *request_) { return PMPI_Win_fence(assert, win); }}, &request, true});
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}

int MPI_Accumulate(const void  *origin_addr,
                   int          origin_count,
                   MPI_Datatype origin_datatype,
                   int          target_rank,
                   MPI_Aint     target_disp,
                   int          target_count,
                   MPI_Datatype target_datatype,
                   MPI_Op       op,
                   MPI_Win      win)
{
    MPI_Request request; // = MPI_REQUEST_NULL;
    oe.post(new mmcso::OffloadCommand{std::function{[=](MPI_Request *request_) {
                                          return PMPI_Accumulate(origin_addr,
                                                                 origin_count,
                                                                 origin_datatype,
                                                                 target_rank,
                                                                 target_disp,
                                                                 target_count,
                                                                 target_datatype,
                                                                 op,
                                                                 win);
                                      }},
                                      &request,
                                      true});
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}
#endif

#if MMCSO_OFFLOAD_NOT_IMPLEMENTED_YET

int MPI_Abort(MPI_Comm comm, int errorcode);
int MPI_Accumulate(const void  *origin_addr,
                   int          origin_count,
                   MPI_Datatype origin_datatype,
                   int          target_rank,
                   MPI_Aint     target_disp,
                   int          target_count,
                   MPI_Datatype target_datatype,
                   MPI_Op       op,
                   MPI_Win      win);
int MPI_Add_error_class(int *errorclass);
int MPI_Add_error_code(int errorclass, int *errorcode);
int MPI_Add_error_string(int errorcode, const char *string);
int MPI_Allgather(const void  *sendbuf,
                  int          sendcount,
                  MPI_Datatype sendtype,
                  void        *recvbuf,
                  int          recvcount,
                  MPI_Datatype recvtype,
                  MPI_Comm     comm);
int MPI_Iallgather(const void  *sendbuf,
                   int          sendcount,
                   MPI_Datatype sendtype,
                   void        *recvbuf,
                   int          recvcount,
                   MPI_Datatype recvtype,
                   MPI_Comm     comm,
                   MPI_Request *request);
int MPI_Allgatherv(const void  *sendbuf,
                   int          sendcount,
                   MPI_Datatype sendtype,
                   void        *recvbuf,
                   const int    recvcounts[],
                   const int    displs[],
                   MPI_Datatype recvtype,
                   MPI_Comm     comm);
int MPI_Iallgatherv(const void  *sendbuf,
                    int          sendcount,
                    MPI_Datatype sendtype,
                    void        *recvbuf,
                    const int    recvcounts[],
                    const int    displs[],
                    MPI_Datatype recvtype,
                    MPI_Comm     comm,
                    MPI_Request *request);
int MPI_Alloc_mem(MPI_Aint size, MPI_Info info, void *baseptr);

int MPI_Alltoall(const void  *sendbuf,
                 int          sendcount,
                 MPI_Datatype sendtype,
                 void        *recvbuf,
                 int          recvcount,
                 MPI_Datatype recvtype,
                 MPI_Comm     comm);
int MPI_Ialltoall(const void  *sendbuf,
                  int          sendcount,
                  MPI_Datatype sendtype,
                  void        *recvbuf,
                  int          recvcount,
                  MPI_Datatype recvtype,
                  MPI_Comm     comm,
                  MPI_Request *request);
int MPI_Alltoallv(const void  *sendbuf,
                  const int    sendcounts[],
                  const int    sdispls[],
                  MPI_Datatype sendtype,
                  void        *recvbuf,
                  const int    recvcounts[],
                  const int    rdispls[],
                  MPI_Datatype recvtype,
                  MPI_Comm     comm);
int MPI_Ialltoallv(const void  *sendbuf,
                   const int    sendcounts[],
                   const int    sdispls[],
                   MPI_Datatype sendtype,
                   void        *recvbuf,
                   const int    recvcounts[],
                   const int    rdispls[],
                   MPI_Datatype recvtype,
                   MPI_Comm     comm,
                   MPI_Request *request);
int MPI_Alltoallw(const void        *sendbuf,
                  const int          sendcounts[],
                  const int          sdispls[],
                  const MPI_Datatype sendtypes[],
                  void              *recvbuf,
                  const int          recvcounts[],
                  const int          rdispls[],
                  const MPI_Datatype recvtypes[],
                  MPI_Comm           comm);
int MPI_Ialltoallw(const void        *sendbuf,
                   const int          sendcounts[],
                   const int          sdispls[],
                   const MPI_Datatype sendtypes[],
                   void              *recvbuf,
                   const int          recvcounts[],
                   const int          rdispls[],
                   const MPI_Datatype recvtypes[],
                   MPI_Comm           comm,
                   MPI_Request       *request);
int MPI_Barrier(MPI_Comm comm);
int MPI_Ibarrier(MPI_Comm comm, MPI_Request *request);
int MPI_Bcast(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm);
int MPI_Bsend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);
int MPI_Ibcast(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm, MPI_Request *request);
int MPI_Bsend_init(
    const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request);
int MPI_Buffer_attach(void *buffer, int size);
int MPI_Buffer_detach(void *buffer, int *size);
int MPI_Cancel(MPI_Request *request);
int MPI_Cart_coords(MPI_Comm comm, int rank, int maxdims, int coords[]);
int MPI_Cart_create(
    MPI_Comm old_comm, int ndims, const int dims[], const int periods[], int reorder, MPI_Comm *comm_cart);
int            MPI_Cart_get(MPI_Comm comm, int maxdims, int dims[], int periods[], int coords[]);
int            MPI_Cart_map(MPI_Comm comm, int ndims, const int dims[], const int periods[], int *newrank);
int            MPI_Cart_rank(MPI_Comm comm, const int coords[], int *rank);
int            MPI_Cart_shift(MPI_Comm comm, int direction, int disp, int *rank_source, int *rank_dest);
int            MPI_Cart_sub(MPI_Comm comm, const int remain_dims[], MPI_Comm *new_comm);
int            MPI_Cartdim_get(MPI_Comm comm, int *ndims);
int            MPI_Close_port(const char *port_name);
int            MPI_Comm_accept(const char *port_name, MPI_Info info, int root, MPI_Comm comm, MPI_Comm *newcomm);
MPI_Fint       MPI_Comm_c2f(MPI_Comm comm);
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
MPI_Comm       MPI_Comm_f2c(MPI_Fint comm);
int            MPI_Comm_free_keyval(int *comm_keyval);
int            MPI_Comm_free(MPI_Comm *comm);
int            MPI_Comm_get_attr(MPI_Comm comm, int comm_keyval, void *attribute_val, int *flag);
int            MPI_Dist_graph_create(MPI_Comm  comm_old,
                                     int       n,
                                     const int nodes[],
                                     const int degrees[],
                                     const int targets[],
                                     const int weights[],
                                     MPI_Info  info,
                                     int       reorder,
                                     MPI_Comm *newcomm);
int            MPI_Dist_graph_create_adjacent(MPI_Comm  comm_old,
                                              int       indegree,
                                              const int sources[],
                                                  const int sourceweights[],
                                              int       outdegree,
                                                  const int destinations[],
                                                  const int destweights[],
                                              MPI_Info  info,
                                              int       reorder,
                                                  MPI_Comm *comm_dist_graph);
int            MPI_Dist_graph_neighbors(MPI_Comm comm,
                                        int      maxindegree,
                                        int      sources[],
                                        int      sourceweights[],
                                        int      maxoutdegree,
                                        int      destinations[],
                                        int      destweights[]);
int            MPI_Dist_graph_neighbors_count(MPI_Comm comm, int *inneighbors, int *outneighbors, int *weighted);
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
int            MPI_Compare_and_swap(const void  *origin_addr,
                                    const void  *compare_addr,
                                    void        *result_addr,
                                    MPI_Datatype datatype,
                                    int          target_rank,
                                    MPI_Aint     target_disp,
                                    MPI_Win      win);
int            MPI_Dims_create(int nnodes, int ndims, int dims[]);
MPI_Fint       MPI_Errhandler_c2f(MPI_Errhandler errhandler);
MPI_Errhandler MPI_Errhandler_f2c(MPI_Fint errhandler);
int            MPI_Errhandler_free(MPI_Errhandler *errhandler);
int            MPI_Error_class(int errorcode, int *errorclass);
int            MPI_Error_string(int errorcode, char *string, int *resultlen);
int      MPI_Exscan(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm);
int      MPI_Fetch_and_op(const void  *origin_addr,
                          void        *result_addr,
                          MPI_Datatype datatype,
                          int          target_rank,
                          MPI_Aint     target_disp,
                          MPI_Op       op,
                          MPI_Win      win);
int      MPI_Iexscan(const void  *sendbuf,
                     void        *recvbuf,
                     int          count,
                     MPI_Datatype datatype,
                     MPI_Op       op,
                     MPI_Comm     comm,
                     MPI_Request *request);
MPI_Fint MPI_File_c2f(MPI_File file);
MPI_File MPI_File_f2c(MPI_Fint file);
int      MPI_File_call_errhandler(MPI_File fh, int errorcode);
int      MPI_File_create_errhandler(MPI_File_errhandler_function *function, MPI_Errhandler *errhandler);
int      MPI_File_set_errhandler(MPI_File file, MPI_Errhandler errhandler);
int      MPI_File_get_errhandler(MPI_File file, MPI_Errhandler *errhandler);
int      MPI_File_open(MPI_Comm comm, const char *filename, int amode, MPI_Info info, MPI_File *fh);
int      MPI_File_close(MPI_File *fh);
int      MPI_File_delete(const char *filename, MPI_Info info);
int      MPI_File_set_size(MPI_File fh, MPI_Offset size);
int      MPI_File_preallocate(MPI_File fh, MPI_Offset size);
int      MPI_File_get_size(MPI_File fh, MPI_Offset *size);
int      MPI_File_get_group(MPI_File fh, MPI_Group *group);
int      MPI_File_get_amode(MPI_File fh, int *amode);
int      MPI_File_set_info(MPI_File fh, MPI_Info info);
int      MPI_File_get_info(MPI_File fh, MPI_Info *info_used);
int      MPI_File_set_view(
         MPI_File fh, MPI_Offset disp, MPI_Datatype etype, MPI_Datatype filetype, const char *datarep, MPI_Info info);
int MPI_File_get_view(MPI_File fh, MPI_Offset *disp, MPI_Datatype *etype, MPI_Datatype *filetype, char *datarep);
int MPI_File_read_at(MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Status *status);
int MPI_File_read_at_all(
    MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Status *status);
int MPI_File_write_at(
    MPI_File fh, MPI_Offset offset, const void *buf, int count, MPI_Datatype datatype, MPI_Status *status);
int MPI_File_write_at_all(
    MPI_File fh, MPI_Offset offset, const void *buf, int count, MPI_Datatype datatype, MPI_Status *status);
int MPI_File_iread_at(
    MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Request *request);
int MPI_File_iwrite_at(
    MPI_File fh, MPI_Offset offset, const void *buf, int count, MPI_Datatype datatype, MPI_Request *request);
int MPI_File_iread_at_all(
    MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Request *request);
int MPI_File_iwrite_at_all(
    MPI_File fh, MPI_Offset offset, const void *buf, int count, MPI_Datatype datatype, MPI_Request *request);
int MPI_File_read(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status);
int MPI_File_read_all(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status);
int MPI_File_write(MPI_File fh, const void *buf, int count, MPI_Datatype datatype, MPI_Status *status);
int MPI_File_write_all(MPI_File fh, const void *buf, int count, MPI_Datatype datatype, MPI_Status *status);
int MPI_File_iread(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Request *request);
int MPI_File_iwrite(MPI_File fh, const void *buf, int count, MPI_Datatype datatype, MPI_Request *request);
int MPI_File_iread_all(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Request *request);
int MPI_File_iwrite_all(MPI_File fh, const void *buf, int count, MPI_Datatype datatype, MPI_Request *request);
int MPI_File_seek(MPI_File fh, MPI_Offset offset, int whence);
int MPI_File_get_position(MPI_File fh, MPI_Offset *offset);
int MPI_File_get_byte_offset(MPI_File fh, MPI_Offset offset, MPI_Offset *disp);
int MPI_File_read_shared(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status);
int MPI_File_write_shared(MPI_File fh, const void *buf, int count, MPI_Datatype datatype, MPI_Status *status);
int MPI_File_iread_shared(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Request *request);
int MPI_File_iwrite_shared(MPI_File fh, const void *buf, int count, MPI_Datatype datatype, MPI_Request *request);
int MPI_File_read_ordered(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status);
int MPI_File_write_ordered(MPI_File fh, const void *buf, int count, MPI_Datatype datatype, MPI_Status *status);
int MPI_File_seek_shared(MPI_File fh, MPI_Offset offset, int whence);
int MPI_File_get_position_shared(MPI_File fh, MPI_Offset *offset);
int MPI_File_read_at_all_begin(MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype);
int MPI_File_read_at_all_end(MPI_File fh, void *buf, MPI_Status *status);
int MPI_File_write_at_all_begin(MPI_File fh, MPI_Offset offset, const void *buf, int count, MPI_Datatype datatype);
int MPI_File_write_at_all_end(MPI_File fh, const void *buf, MPI_Status *status);
int MPI_File_read_all_begin(MPI_File fh, void *buf, int count, MPI_Datatype datatype);
int MPI_File_read_all_end(MPI_File fh, void *buf, MPI_Status *status);
int MPI_File_write_all_begin(MPI_File fh, const void *buf, int count, MPI_Datatype datatype);
int MPI_File_write_all_end(MPI_File fh, const void *buf, MPI_Status *status);
int MPI_File_read_ordered_begin(MPI_File fh, void *buf, int count, MPI_Datatype datatype);
int MPI_File_read_ordered_end(MPI_File fh, void *buf, MPI_Status *status);
int MPI_File_write_ordered_begin(MPI_File fh, const void *buf, int count, MPI_Datatype datatype);
int MPI_File_write_ordered_end(MPI_File fh, const void *buf, MPI_Status *status);
int MPI_File_get_type_extent(MPI_File fh, MPI_Datatype datatype, MPI_Aint *extent);
int MPI_File_set_atomicity(MPI_File fh, int flag);
int MPI_File_get_atomicity(MPI_File fh, int *flag);
int MPI_File_sync(MPI_File fh);
int MPI_Finalize(void);
int MPI_Finalized(int *flag);
int MPI_Free_mem(void *base);
int MPI_Gather(const void  *sendbuf,
               int          sendcount,
               MPI_Datatype sendtype,
               void        *recvbuf,
               int          recvcount,
               MPI_Datatype recvtype,
               int          root,
               MPI_Comm     comm);
int MPI_Igather(const void  *sendbuf,
                int          sendcount,
                MPI_Datatype sendtype,
                void        *recvbuf,
                int          recvcount,
                MPI_Datatype recvtype,
                int          root,
                MPI_Comm     comm,
                MPI_Request *request);
int MPI_Gatherv(const void  *sendbuf,
                int          sendcount,
                MPI_Datatype sendtype,
                void        *recvbuf,
                const int    recvcounts[],
                const int    displs[],
                MPI_Datatype recvtype,
                int          root,
                MPI_Comm     comm);
int MPI_Igatherv(const void  *sendbuf,
                 int          sendcount,
                 MPI_Datatype sendtype,
                 void        *recvbuf,
                 const int    recvcounts[],
                 const int    displs[],
                 MPI_Datatype recvtype,
                 int          root,
                 MPI_Comm     comm,
                 MPI_Request *request);
int MPI_Get_address(const void *location, MPI_Aint *address);
int MPI_Get_count(const MPI_Status *status, MPI_Datatype datatype, int *count);
int MPI_Get_elements(const MPI_Status *status, MPI_Datatype datatype, int *count);
int MPI_Get_elements_x(const MPI_Status *status, MPI_Datatype datatype, MPI_Count *count);
int MPI_Get(void        *origin_addr,
            int          origin_count,
            MPI_Datatype origin_datatype,
            int          target_rank,
            MPI_Aint     target_disp,
            int          target_count,
            MPI_Datatype target_datatype,
            MPI_Win      win);
int MPI_Get_accumulate(const void  *origin_addr,
                       int          origin_count,
                       MPI_Datatype origin_datatype,
                       void        *result_addr,
                       int          result_count,
                       MPI_Datatype result_datatype,
                       int          target_rank,
                       MPI_Aint     target_disp,
                       int          target_count,
                       MPI_Datatype target_datatype,
                       MPI_Op       op,
                       MPI_Win      win);
int MPI_Get_library_version(char *version, int *resultlen);
int MPI_Get_processor_name(char *name, int *resultlen);
int MPI_Get_version(int *version, int *subversion);
int MPI_Graph_create(
    MPI_Comm comm_old, int nnodes, const int index[], const int edges[], int reorder, MPI_Comm *comm_graph);
int       MPI_Graph_get(MPI_Comm comm, int maxindex, int maxedges, int index[], int edges[]);
int       MPI_Graph_map(MPI_Comm comm, int nnodes, const int index[], const int edges[], int *newrank);
int       MPI_Graph_neighbors_count(MPI_Comm comm, int rank, int *nneighbors);
int       MPI_Graph_neighbors(MPI_Comm comm, int rank, int maxneighbors, int neighbors[]);
int       MPI_Graphdims_get(MPI_Comm comm, int *nnodes, int *nedges);
int       MPI_Grequest_complete(MPI_Request request);
int       MPI_Grequest_start(MPI_Grequest_query_function  *query_fn,
                             MPI_Grequest_free_function   *free_fn,
                                      MPI_Grequest_cancel_function *cancel_fn,
                             void                         *extra_state,
                             MPI_Request                  *request);
MPI_Fint  MPI_Group_c2f(MPI_Group group);
int       MPI_Group_compare(MPI_Group group1, MPI_Group group2, int *result);
int       MPI_Group_difference(MPI_Group group1, MPI_Group group2, MPI_Group *newgroup);
int       MPI_Group_excl(MPI_Group group, int n, const int ranks[], MPI_Group *newgroup);
MPI_Group MPI_Group_f2c(MPI_Fint group);
int       MPI_Group_free(MPI_Group *group);
int       MPI_Group_incl(MPI_Group group, int n, const int ranks[], MPI_Group *newgroup);
int       MPI_Group_intersection(MPI_Group group1, MPI_Group group2, MPI_Group *newgroup);
int       MPI_Group_range_excl(MPI_Group group, int n, int ranges[][3], MPI_Group *newgroup);
int       MPI_Group_range_incl(MPI_Group group, int n, int ranges[][3], MPI_Group *newgroup);
int       MPI_Group_rank(MPI_Group group, int *rank);
int       MPI_Group_size(MPI_Group group, int *size);
int       MPI_Group_translate_ranks(MPI_Group group1, int n, const int ranks1[], MPI_Group group2, int ranks2[]);
int       MPI_Group_union(MPI_Group group1, MPI_Group group2, MPI_Group *newgroup);
int       MPI_Ibsend(
          const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request);
int      MPI_Improbe(int source, int tag, MPI_Comm comm, int *flag, MPI_Message *message, MPI_Status *status);
int      MPI_Imrecv(void *buf, int count, MPI_Datatype type, MPI_Message *message, MPI_Request *request);
MPI_Fint MPI_Info_c2f(MPI_Info info);
int      MPI_Info_create(MPI_Info *info);
int      MPI_Info_delete(MPI_Info info, const char *key);
int      MPI_Info_dup(MPI_Info info, MPI_Info *newinfo);
MPI_Info MPI_Info_f2c(MPI_Fint info);
int      MPI_Info_free(MPI_Info *info);
int      MPI_Info_get(MPI_Info info, const char *key, int valuelen, char *value, int *flag);
int      MPI_Info_get_nkeys(MPI_Info info, int *nkeys);
int      MPI_Info_get_nthkey(MPI_Info info, int n, char *key);
int      MPI_Info_get_valuelen(MPI_Info info, const char *key, int *valuelen, int *flag);
int      MPI_Info_set(MPI_Info info, const char *key, const char *value);
int      MPI_Init(int *argc, char ***argv);
int      MPI_Initialized(int *flag);
int      MPI_Init_thread(int *argc, char ***argv, int required, int *provided);
int      MPI_Intercomm_create(
         MPI_Comm local_comm, int local_leader, MPI_Comm bridge_comm, int remote_leader, int tag, MPI_Comm *newintercomm);
int MPI_Intercomm_merge(MPI_Comm intercomm, int high, MPI_Comm *newintercomm);
int MPI_Iprobe(int source, int tag, MPI_Comm comm, int *flag, MPI_Status *status);
int MPI_Irecv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Request *request);
int MPI_Irsend(
    const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request);
int MPI_Isend(
    const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request);
int MPI_Issend(
    const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request);
int         MPI_Is_thread_main(int *flag);
int         MPI_Lookup_name(const char *service_name, MPI_Info info, char *port_name);
MPI_Fint    MPI_Message_c2f(MPI_Message message);
MPI_Message MPI_Message_f2c(MPI_Fint message);
int         MPI_Mprobe(int source, int tag, MPI_Comm comm, MPI_Message *message, MPI_Status *status);
int         MPI_Mrecv(void *buf, int count, MPI_Datatype type, MPI_Message *message, MPI_Status *status);
int         MPI_Neighbor_allgather(const void  *sendbuf,
                                   int          sendcount,
                                   MPI_Datatype sendtype,
                                   void        *recvbuf,
                                   int          recvcount,
                                   MPI_Datatype recvtype,
                                   MPI_Comm     comm);
int         MPI_Ineighbor_allgather(const void  *sendbuf,
                                    int          sendcount,
                                    MPI_Datatype sendtype,
                                    void        *recvbuf,
                                    int          recvcount,
                                    MPI_Datatype recvtype,
                                    MPI_Comm     comm,
                                    MPI_Request *request);
int         MPI_Neighbor_allgatherv(const void  *sendbuf,
                                    int          sendcount,
                                    MPI_Datatype sendtype,
                                    void        *recvbuf,
                                    const int    recvcounts[],
                                    const int    displs[],
                                    MPI_Datatype recvtype,
                                    MPI_Comm     comm);
int         MPI_Ineighbor_allgatherv(const void  *sendbuf,
                                     int          sendcount,
                                     MPI_Datatype sendtype,
                                     void        *recvbuf,
                                     const int    recvcounts[],
                                     const int    displs[],
                                     MPI_Datatype recvtype,
                                     MPI_Comm     comm,
                                     MPI_Request *request);
int         MPI_Neighbor_alltoall(const void  *sendbuf,
                                  int          sendcount,
                                  MPI_Datatype sendtype,
                                  void        *recvbuf,
                                  int          recvcount,
                                  MPI_Datatype recvtype,
                                  MPI_Comm     comm);
int         MPI_Ineighbor_alltoall(const void  *sendbuf,
                                   int          sendcount,
                                   MPI_Datatype sendtype,
                                   void        *recvbuf,
                                   int          recvcount,
                                   MPI_Datatype recvtype,
                                   MPI_Comm     comm,
                                   MPI_Request *request);
int         MPI_Neighbor_alltoallv(const void  *sendbuf,
                                   const int    sendcounts[],
                                   const int    sdispls[],
                                   MPI_Datatype sendtype,
                                   void        *recvbuf,
                                   const int    recvcounts[],
                                   const int    rdispls[],
                                   MPI_Datatype recvtype,
                                   MPI_Comm     comm);
int         MPI_Ineighbor_alltoallv(const void  *sendbuf,
                                    const int    sendcounts[],
                                    const int    sdispls[],
                                    MPI_Datatype sendtype,
                                    void        *recvbuf,
                                    const int    recvcounts[],
                                    const int    rdispls[],
                                    MPI_Datatype recvtype,
                                    MPI_Comm     comm,
                                    MPI_Request *request);
int         MPI_Neighbor_alltoallw(const void        *sendbuf,
                                   const int          sendcounts[],
                                   const MPI_Aint     sdispls[],
                                   const MPI_Datatype sendtypes[],
                                   void              *recvbuf,
                                   const int          recvcounts[],
                                   const MPI_Aint     rdispls[],
                                   const MPI_Datatype recvtypes[],
                                   MPI_Comm           comm);
int         MPI_Ineighbor_alltoallw(const void        *sendbuf,
                                    const int          sendcounts[],
                                    const MPI_Aint     sdispls[],
                                    const MPI_Datatype sendtypes[],
                                    void              *recvbuf,
                                    const int          recvcounts[],
                                    const MPI_Aint     rdispls[],
                                    const MPI_Datatype recvtypes[],
                                    MPI_Comm           comm,
                                    MPI_Request       *request);
MPI_Fint    MPI_Op_c2f(MPI_Op op);
int         MPI_Op_commutative(MPI_Op op, int *commute);
int         MPI_Op_create(MPI_User_function *function, int commute, MPI_Op *op);
int         MPI_Open_port(MPI_Info info, char *port_name);
MPI_Op      MPI_Op_f2c(MPI_Fint op);
int         MPI_Op_free(MPI_Op *op);
int         MPI_Pack_external(const char   datarep[],
                              const void  *inbuf,
                              int          incount,
                              MPI_Datatype datatype,
                              void        *outbuf,
                              MPI_Aint     outsize,
                              MPI_Aint    *position);
int         MPI_Pack_external_size(const char datarep[], int incount, MPI_Datatype datatype, MPI_Aint *size);
int         MPI_Pack(
            const void *inbuf, int incount, MPI_Datatype datatype, void *outbuf, int outsize, int *position, MPI_Comm comm);
int MPI_Pack_size(int incount, MPI_Datatype datatype, MPI_Comm comm, int *size);
int MPI_Pcontrol(const int level, ...);
int MPI_Probe(int source, int tag, MPI_Comm comm, MPI_Status *status);
int MPI_Publish_name(const char *service_name, MPI_Info info, const char *port_name);
int MPI_Put(const void  *origin_addr,
            int          origin_count,
            MPI_Datatype origin_datatype,
            int          target_rank,
            MPI_Aint     target_disp,
            int          target_count,
            MPI_Datatype target_datatype,
            MPI_Win      win);
int MPI_Query_thread(int *provided);
int MPI_Raccumulate(const void  *origin_addr,
                    int          origin_count,
                    MPI_Datatype origin_datatype,
                    int          target_rank,
                    MPI_Aint     target_disp,
                    int          target_count,
                    MPI_Datatype target_datatype,
                    MPI_Op       op,
                    MPI_Win      win,
                    MPI_Request *request);
int MPI_Recv_init(
    void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Request *request);
int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status);
int MPI_Reduce(
    const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm);
int MPI_Ireduce(const void  *sendbuf,
                void        *recvbuf,
                int          count,
                MPI_Datatype datatype,
                MPI_Op       op,
                int          root,
                MPI_Comm     comm,
                MPI_Request *request);
int MPI_Reduce_local(const void *inbuf, void *inoutbuf, int count, MPI_Datatype datatype, MPI_Op op);
int MPI_Reduce_scatter(
    const void *sendbuf, void *recvbuf, const int recvcounts[], MPI_Datatype datatype, MPI_Op op, MPI_Comm comm);
int MPI_Ireduce_scatter(const void  *sendbuf,
                        void        *recvbuf,
                        const int    recvcounts[],
                        MPI_Datatype datatype,
                        MPI_Op       op,
                        MPI_Comm     comm,
                        MPI_Request *request);
int MPI_Reduce_scatter_block(
    const void *sendbuf, void *recvbuf, int recvcount, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm);
int         MPI_Ireduce_scatter_block(const void  *sendbuf,
                                      void        *recvbuf,
                                      int          recvcount,
                                      MPI_Datatype datatype,
                                      MPI_Op       op,
                                      MPI_Comm     comm,
                                      MPI_Request *request);
int         MPI_Register_datarep(const char                      *datarep,
                                        MPI_Datarep_conversion_function *read_conversion_fn,
                                        MPI_Datarep_conversion_function *write_conversion_fn,
                                 MPI_Datarep_extent_function     *dtype_file_extent_fn,
                                 void                            *extra_state);
MPI_Fint    MPI_Request_c2f(MPI_Request request);
MPI_Request MPI_Request_f2c(MPI_Fint request);
int         MPI_Request_free(MPI_Request *request);
int         MPI_Request_get_status(MPI_Request request, int *flag, MPI_Status *status);
int         MPI_Rget(void        *origin_addr,
                     int          origin_count,
                     MPI_Datatype origin_datatype,
                     int          target_rank,
                     MPI_Aint     target_disp,
                     int          target_count,
                     MPI_Datatype target_datatype,
                     MPI_Win      win,
                                  MPI_Request *request);
int         MPI_Rget_accumulate(const void  *origin_addr,
                                int          origin_count,
                                MPI_Datatype origin_datatype,
                                void        *result_addr,
                                int          result_count,
                                MPI_Datatype result_datatype,
                                int          target_rank,
                                MPI_Aint     target_disp,
                                int          target_count,
                                MPI_Datatype target_datatype,
                                MPI_Op       op,
                                MPI_Win      win,
                                MPI_Request *request);
int         MPI_Rput(const void  *origin_addr,
                     int          origin_count,
                     MPI_Datatype origin_datatype,
                     int          target_rank,
                     MPI_Aint     target_disp,
                     int          target_cout,
                     MPI_Datatype target_datatype,
                     MPI_Win      win,
                     MPI_Request *request);
int         MPI_Rsend(const void *ibuf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);
int         MPI_Rsend_init(
            const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request);
int MPI_Scan(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm);
int MPI_Iscan(const void  *sendbuf,
              void        *recvbuf,
              int          count,
              MPI_Datatype datatype,
              MPI_Op       op,
              MPI_Comm     comm,
              MPI_Request *request);
int MPI_Scatter(const void  *sendbuf,
                int          sendcount,
                MPI_Datatype sendtype,
                void        *recvbuf,
                int          recvcount,
                MPI_Datatype recvtype,
                int          root,
                MPI_Comm     comm);
int MPI_Iscatter(const void  *sendbuf,
                 int          sendcount,
                 MPI_Datatype sendtype,
                 void        *recvbuf,
                 int          recvcount,
                 MPI_Datatype recvtype,
                 int          root,
                 MPI_Comm     comm,
                                 MPI_Request *request);
int MPI_Scatterv(const void  *sendbuf,
                 const int    sendcounts[],
                 const int    displs[],
                 MPI_Datatype sendtype,
                 void        *recvbuf,
                 int          recvcount,
                 MPI_Datatype recvtype,
                 int          root,
                 MPI_Comm     comm);
int MPI_Iscatterv(const void  *sendbuf,
                  const int    sendcounts[],
                  const int    displs[],
                  MPI_Datatype sendtype,
                  void        *recvbuf,
                  int          recvcount,
                  MPI_Datatype recvtype,
                  int          root,
                  MPI_Comm     comm,
                                  MPI_Request *request);
int MPI_Send_init(
    const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request);
int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);
int MPI_Sendrecv(const void  *sendbuf,
                 int          sendcount,
                 MPI_Datatype sendtype,
                 int          dest,
                 int          sendtag,
                 void        *recvbuf,
                 int          recvcount,
                 MPI_Datatype recvtype,
                 int          source,
                 int          recvtag,
                 MPI_Comm     comm,
                 MPI_Status  *status);
int MPI_Sendrecv_replace(void        *buf,
                         int          count,
                         MPI_Datatype datatype,
                         int          dest,
                         int          sendtag,
                         int          source,
                         int          recvtag,
                         MPI_Comm     comm,
                         MPI_Status  *status);
int MPI_Ssend_init(
    const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request);
int      MPI_Ssend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);
int      MPI_Start(MPI_Request *request);
int      MPI_Startall(int count, MPI_Request array_of_requests[]);
int      MPI_Status_c2f(const MPI_Status *c_status, MPI_Fint *f_status);
int      MPI_Status_f2c(const MPI_Fint *f_status, MPI_Status *c_status);
int      MPI_Status_set_cancelled(MPI_Status *status, int flag);
int      MPI_Status_set_elements(MPI_Status *status, MPI_Datatype datatype, int count);
int      MPI_Status_set_elements_x(MPI_Status *status, MPI_Datatype datatype, MPI_Count count);

int      MPI_Topo_test(MPI_Comm comm, int *status);
MPI_Fint MPI_Type_c2f(MPI_Datatype datatype);
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
MPI_Datatype MPI_Type_f2c(MPI_Fint datatype);
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
int MPI_Unpack(
    const void *inbuf, int insize, int *position, void *outbuf, int outcount, MPI_Datatype datatype, MPI_Comm comm);
int MPI_Unpublish_name(const char *service_name, MPI_Info info, const char *port_name);
int MPI_Unpack_external(const char   datarep[],
                        const void  *inbuf,
                        MPI_Aint     insize,
                        MPI_Aint    *position,
                        void        *outbuf,
                        int          outcount,
                                        MPI_Datatype datatype);

int MPI_Win_allocate(MPI_Aint size, int disp_unit, MPI_Info info, MPI_Comm comm, void *baseptr, MPI_Win *win);
int MPI_Win_allocate_shared(MPI_Aint size, int disp_unit, MPI_Info info, MPI_Comm comm, void *baseptr, MPI_Win *win);
int MPI_Win_attach(MPI_Win win, void *base, MPI_Aint size);
MPI_Fint MPI_Win_c2f(MPI_Win win);
int      MPI_Win_call_errhandler(MPI_Win win, int errorcode);
int      MPI_Win_complete(MPI_Win win);
int      MPI_Win_create(void *base, MPI_Aint size, int disp_unit, MPI_Info info, MPI_Comm comm, MPI_Win *win);
int      MPI_Win_create_dynamic(MPI_Info info, MPI_Comm comm, MPI_Win *win);
int      MPI_Win_create_errhandler(MPI_Win_errhandler_function *function, MPI_Errhandler *errhandler);
int      MPI_Win_create_keyval(MPI_Win_copy_attr_function   *win_copy_attr_fn,
                                         MPI_Win_delete_attr_function *win_delete_attr_fn,
                               int                          *win_keyval,
                               void                         *extra_state);
int      MPI_Win_delete_attr(MPI_Win win, int win_keyval);
int      MPI_Win_detach(MPI_Win win, const void *base);
MPI_Win  MPI_Win_f2c(MPI_Fint win);
int      MPI_Win_fence(int assert, MPI_Win win);
int      MPI_Win_flush(int rank, MPI_Win win);
int      MPI_Win_flush_all(MPI_Win win);
int      MPI_Win_flush_local(int rank, MPI_Win win);
int      MPI_Win_flush_local_all(MPI_Win win);
int      MPI_Win_free(MPI_Win *win);
int      MPI_Win_free_keyval(int *win_keyval);
int      MPI_Win_get_attr(MPI_Win win, int win_keyval, void *attribute_val, int *flag);
int      MPI_Win_get_errhandler(MPI_Win win, MPI_Errhandler *errhandler);
int      MPI_Win_get_group(MPI_Win win, MPI_Group *group);
int      MPI_Win_get_info(MPI_Win win, MPI_Info *info_used);
int      MPI_Win_get_name(MPI_Win win, char *win_name, int *resultlen);
int      MPI_Win_lock(int lock_type, int rank, int assert, MPI_Win win);
int      MPI_Win_lock_all(int assert, MPI_Win win);
int      MPI_Win_post(MPI_Group group, int assert, MPI_Win win);
int      MPI_Win_set_attr(MPI_Win win, int win_keyval, void *attribute_val);
int      MPI_Win_set_errhandler(MPI_Win win, MPI_Errhandler errhandler);
int      MPI_Win_set_info(MPI_Win win, MPI_Info info);
int      MPI_Win_set_name(MPI_Win win, const char *win_name);
int      MPI_Win_shared_query(MPI_Win win, int rank, MPI_Aint *size, int *disp_unit, void *baseptr);
int      MPI_Win_start(MPI_Group group, int assert, MPI_Win win);
int      MPI_Win_sync(MPI_Win win);
int      MPI_Win_test(MPI_Win win, int *flag);
int      MPI_Win_unlock(int rank, MPI_Win win);
int      MPI_Win_unlock_all(MPI_Win win);
int      MPI_Win_wait(MPI_Win win);
double   MPI_Wtick(void);
double   MPI_Wtime(void);

#endif /* MMCSO_OFFLOAD_NOT_IMPLEMENTED_YET */

#if 0

int MPI_Iallgather(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                                 void *recvbuf, int recvcount,
                                 MPI_Datatype recvtype, MPI_Comm comm, MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{std::function{[=](MPI_Request *request_) {
                                   return PMPI_Iallgather(
                                       sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, request_);
                               }},
                                      request});
    return MPI_SUCCESS;
}

int MPI_Allgather(const void *sendbuf,  int sendcount,  MPI_Datatype sendtype, 
                                 void *recvbuf,  int recvcount, 
                                 MPI_Datatype recvtype,  MPI_Comm comm)
{
    MPI_Request request;
    MPI_Iallgather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}

#endif

int MPI_Iallgatherv(const void  *sendbuf,
                    int          sendcount,
                    MPI_Datatype sendtype,
                    void        *recvbuf,
                    const int    recvcounts[],
                    const int    displs[],
                    MPI_Datatype recvtype,
                    MPI_Comm     comm,
                    MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{
        std::function{[=](MPI_Request *request_) {
            return PMPI_Iallgatherv(
                sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm, request_);
        }},
        request});
    return MPI_SUCCESS;
}

int MPI_Allgatherv(const void  *sendbuf,
                   int          sendcount,
                   MPI_Datatype sendtype,
                   void        *recvbuf,
                   const int    recvcounts[],
                   const int    displs[],
                   MPI_Datatype recvtype,
                   MPI_Comm     comm)
{
    MPI_Request request;
    MPI_Iallgatherv(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}

int MPI_Ialltoall(const void  *sendbuf,
                  int          sendcount,
                  MPI_Datatype sendtype,
                  void        *recvbuf,
                  int          recvcount,
                  MPI_Datatype recvtype,
                  MPI_Comm     comm,
                  MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{
        std::function{[=](MPI_Request *request_) {
            return PMPI_Ialltoall(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, request_);
        }},
        request});
    return MPI_SUCCESS;
}

int MPI_Alltoall(const void  *sendbuf,
                 int          sendcount,
                 MPI_Datatype sendtype,
                 void        *recvbuf,
                 int          recvcount,
                 MPI_Datatype recvtype,
                 MPI_Comm     comm)
{
    MPI_Request request;
    MPI_Ialltoall(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}

int MPI_Ialltoallv(const void  *sendbuf,
                   const int    sendcounts[],
                   const int    sdispls[],
                   MPI_Datatype sendtype,
                   void        *recvbuf,
                   const int    recvcounts[],
                   const int    rdispls[],
                   MPI_Datatype recvtype,
                   MPI_Comm     comm,
                   MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{
        std::function{[=](MPI_Request *request_) {
            return PMPI_Ialltoallv(
                sendbuf, sendcounts, sdispls, sendtype, recvbuf, recvcounts, rdispls, recvtype, comm, request_);
        }},
        request});
    return MPI_SUCCESS;
}

int MPI_Alltoallv(const void  *sendbuf,
                  const int    sendcounts[],
                  const int    sdispls[],
                  MPI_Datatype sendtype,
                  void        *recvbuf,
                  const int    recvcounts[],
                  const int    rdispls[],
                  MPI_Datatype recvtype,
                  MPI_Comm     comm)
{
    MPI_Request request;
    MPI_Ialltoallv(sendbuf, sendcounts, sdispls, sendtype, recvbuf, recvcounts, rdispls, recvtype, comm, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}

int MPI_Ialltoallw(const void        *sendbuf,
                   const int          sendcounts[],
                   const int          sdispls[],
                   const MPI_Datatype sendtypes[],
                   void              *recvbuf,
                   const int          recvcounts[],
                   const int          rdispls[],
                   const MPI_Datatype recvtypes[],
                   MPI_Comm           comm,
                   MPI_Request       *request)
{
    oe.post(new mmcso::OffloadCommand{
        std::function{[=](MPI_Request *request_) {
            return PMPI_Ialltoallw(
                sendbuf, sendcounts, sdispls, sendtypes, recvbuf, recvcounts, rdispls, recvtypes, comm, request_);
        }},
        request});
    return MPI_SUCCESS;
}

int MPI_Alltoallw(const void        *sendbuf,
                  const int          sendcounts[],
                  const int          sdispls[],
                  const MPI_Datatype sendtypes[],
                  void              *recvbuf,
                  const int          recvcounts[],
                  const int          rdispls[],
                  const MPI_Datatype recvtypes[],
                  MPI_Comm           comm)
{
    MPI_Request request;
    MPI_Ialltoallw(sendbuf, sendcounts, sdispls, sendtypes, recvbuf, recvcounts, rdispls, recvtypes, comm, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}

#if 0
int MPI_Ibarrier(MPI_Comm comm, MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{std::function{[=](MPI_Request *request_) {
                                   return PMPI_Ibarrier(
                                       comm, request_);
                               }},
                                      request});
    return MPI_SUCCESS;
}

int MPI_Barrier(MPI_Comm comm)
{
    MPI_Request request;
    MPI_Ibarrier(comm, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}
#endif

#if 0
int MPI_Ibcast(void *buffer, int count, MPI_Datatype datatype,
				                              int root, MPI_Comm comm,
											  MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{std::function{[=](MPI_Request *request_) {
                                   return PMPI_Ibcast(
                                       buffer, count, datatype, root, comm, request_);
                               }},
                                      request});
    return MPI_SUCCESS;
}

int MPI_Bcast(void *buffer,  int count,  MPI_Datatype datatype, 
				                              int root,  MPI_Comm comm)
{
    MPI_Request request;
    MPI_Ibcast(buffer, count, datatype, root, comm, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}
#endif

int MPI_Iexscan(const void  *sendbuf,
                void        *recvbuf,
                int          count,
                MPI_Datatype datatype,
                MPI_Op       op,
                MPI_Comm     comm,
                MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{std::function{[=](MPI_Request *request_) {
                                          return PMPI_Iexscan(sendbuf, recvbuf, count, datatype, op, comm, request_);
                                      }},
                                      request});
    return MPI_SUCCESS;
}

int MPI_Exscan(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm)
{
    MPI_Request request;
    MPI_Iexscan(sendbuf, recvbuf, count, datatype, op, comm, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}

int MPI_Igather(const void  *sendbuf,
                int          sendcount,
                MPI_Datatype sendtype,
                void        *recvbuf,
                int          recvcount,
                MPI_Datatype recvtype,
                int          root,
                MPI_Comm     comm,
                MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{
        std::function{[=](MPI_Request *request_) {
            return PMPI_Igather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm, request_);
        }},
        request});
    return MPI_SUCCESS;
}

int MPI_Gather(const void  *sendbuf,
               int          sendcount,
               MPI_Datatype sendtype,
               void        *recvbuf,
               int          recvcount,
               MPI_Datatype recvtype,
               int          root,
               MPI_Comm     comm)
{
    MPI_Request request;
    MPI_Igather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}

int MPI_Igatherv(const void  *sendbuf,
                 int          sendcount,
                 MPI_Datatype sendtype,
                 void        *recvbuf,
                 const int    recvcounts[],
                 const int    displs[],
                 MPI_Datatype recvtype,
                 int          root,
                 MPI_Comm     comm,
                 MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{
        std::function{[=](MPI_Request *request_) {
            return PMPI_Igatherv(
                sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, root, comm, request_);
        }},
        request});
    return MPI_SUCCESS;
}

int MPI_Gatherv(const void  *sendbuf,
                int          sendcount,
                MPI_Datatype sendtype,
                void        *recvbuf,
                const int    recvcounts[],
                const int    displs[],
                MPI_Datatype recvtype,
                int          root,
                MPI_Comm     comm)
{
    MPI_Request request;
    MPI_Igatherv(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, root, comm, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}

int MPI_Ibsend(
    const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{std::function{[=](MPI_Request *request_) {
                                          return PMPI_Ibsend(buf, count, datatype, dest, tag, comm, request_);
                                      }},
                                      request});
    return MPI_SUCCESS;
}

int MPI_Bsend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
{
    MPI_Request request;
    MPI_Ibsend(buf, count, datatype, dest, tag, comm, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}

int MPI_Imrecv(void *buf, int count, MPI_Datatype type, MPI_Message *message, MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{
        std::function{[=](MPI_Request *request_) { return PMPI_Imrecv(buf, count, type, message, request_); }},
        request});
    return MPI_SUCCESS;
}

#if 0
int MPI_Mrecv(void *buf,  int count,  MPI_Datatype type, 
                              MPI_Message *message)
{
    MPI_Request request;
    MPI_Imrecv(buf, count, type, message, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}
#endif

int MPI_Irsend(
    const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{std::function{[=](MPI_Request *request_) {
                                          return PMPI_Irsend(buf, count, datatype, dest, tag, comm, request_);
                                      }},
                                      request});
    return MPI_SUCCESS;
}

int MPI_Rsend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
{
    MPI_Request request;
    MPI_Irsend(buf, count, datatype, dest, tag, comm, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}

int MPI_Issend(
    const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{std::function{[=](MPI_Request *request_) {
                                          return PMPI_Issend(buf, count, datatype, dest, tag, comm, request_);
                                      }},
                                      request});
    return MPI_SUCCESS;
}

int MPI_Ssend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
{
    MPI_Request request;
    MPI_Issend(buf, count, datatype, dest, tag, comm, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}

int MPI_Ineighbor_allgather(const void  *sendbuf,
                            int          sendcount,
                            MPI_Datatype sendtype,
                            void        *recvbuf,
                            int          recvcount,
                            MPI_Datatype recvtype,
                            MPI_Comm     comm,
                            MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{
        std::function{[=](MPI_Request *request_) {
            return PMPI_Ineighbor_allgather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, request_);
        }},
        request});
    return MPI_SUCCESS;
}

int MPI_Neighbor_allgather(const void  *sendbuf,
                           int          sendcount,
                           MPI_Datatype sendtype,
                           void        *recvbuf,
                           int          recvcount,
                           MPI_Datatype recvtype,
                           MPI_Comm     comm)
{
    MPI_Request request;
    MPI_Ineighbor_allgather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}

int MPI_Ineighbor_allgatherv(const void  *sendbuf,
                             int          sendcount,
                             MPI_Datatype sendtype,
                             void        *recvbuf,
                             const int    recvcounts[],
                             const int    displs[],
                             MPI_Datatype recvtype,
                             MPI_Comm     comm,
                             MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{
        std::function{[=](MPI_Request *request_) {
            return PMPI_Ineighbor_allgatherv(
                sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm, request_);
        }},
        request});
    return MPI_SUCCESS;
}

int MPI_Neighbor_allgatherv(const void  *sendbuf,
                            int          sendcount,
                            MPI_Datatype sendtype,
                            void        *recvbuf,
                            const int    recvcounts[],
                            const int    displs[],
                            MPI_Datatype recvtype,
                            MPI_Comm     comm)
{
    MPI_Request request;
    MPI_Ineighbor_allgatherv(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}

int MPI_Ineighbor_alltoall(const void  *sendbuf,
                           int          sendcount,
                           MPI_Datatype sendtype,
                           void        *recvbuf,
                           int          recvcount,
                           MPI_Datatype recvtype,
                           MPI_Comm     comm,
                           MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{
        std::function{[=](MPI_Request *request_) {
            return PMPI_Ineighbor_alltoall(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, request_);
        }},
        request});
    return MPI_SUCCESS;
}

int MPI_Neighbor_alltoall(const void  *sendbuf,
                          int          sendcount,
                          MPI_Datatype sendtype,
                          void        *recvbuf,
                          int          recvcount,
                          MPI_Datatype recvtype,
                          MPI_Comm     comm)
{
    MPI_Request request;
    MPI_Ineighbor_alltoall(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}

int MPI_Ineighbor_alltoallv(const void  *sendbuf,
                            const int    sendcounts[],
                            const int    sdispls[],
                            MPI_Datatype sendtype,
                            void        *recvbuf,
                            const int    recvcounts[],
                            const int    rdispls[],
                            MPI_Datatype recvtype,
                            MPI_Comm     comm,
                            MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{
        std::function{[=](MPI_Request *request_) {
            return PMPI_Ineighbor_alltoallv(
                sendbuf, sendcounts, sdispls, sendtype, recvbuf, recvcounts, rdispls, recvtype, comm, request_);
        }},
        request});
    return MPI_SUCCESS;
}

int MPI_Neighbor_alltoallv(const void  *sendbuf,
                           const int    sendcounts[],
                           const int    sdispls[],
                           MPI_Datatype sendtype,
                           void        *recvbuf,
                           const int    recvcounts[],
                           const int    rdispls[],
                           MPI_Datatype recvtype,
                           MPI_Comm     comm)
{
    MPI_Request request;
    MPI_Ineighbor_alltoallv(
        sendbuf, sendcounts, sdispls, sendtype, recvbuf, recvcounts, rdispls, recvtype, comm, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}

int MPI_Ineighbor_alltoallw(const void        *sendbuf,
                            const int          sendcounts[],
                            const MPI_Aint     sdispls[],
                            const MPI_Datatype sendtypes[],
                            void              *recvbuf,
                            const int          recvcounts[],
                            const MPI_Aint     rdispls[],
                            const MPI_Datatype recvtypes[],
                            MPI_Comm           comm,
                            MPI_Request       *request)
{
    oe.post(new mmcso::OffloadCommand{
        std::function{[=](MPI_Request *request_) {
            return PMPI_Ineighbor_alltoallw(
                sendbuf, sendcounts, sdispls, sendtypes, recvbuf, recvcounts, rdispls, recvtypes, comm, request_);
        }},
        request});
    return MPI_SUCCESS;
}

int MPI_Neighbor_alltoallw(const void        *sendbuf,
                           const int          sendcounts[],
                           const MPI_Aint     sdispls[],
                           const MPI_Datatype sendtypes[],
                           void              *recvbuf,
                           const int          recvcounts[],
                           const MPI_Aint     rdispls[],
                           const MPI_Datatype recvtypes[],
                           MPI_Comm           comm)
{
    MPI_Request request;
    MPI_Ineighbor_alltoallw(
        sendbuf, sendcounts, sdispls, sendtypes, recvbuf, recvcounts, rdispls, recvtypes, comm, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}

#if 0
int MPI_Ireduce(const void *sendbuf, void *recvbuf, int count,
                              MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm, MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{std::function{[=](MPI_Request *request_) {
                                   return PMPI_Ireduce(
                                       sendbuf, recvbuf, count, datatype, op, root, comm, request_);
                               }},
                                      request});
    return MPI_SUCCESS;
}

int MPI_Reduce(const void *sendbuf,  void *recvbuf,  int count, 
                              MPI_Datatype datatype,  MPI_Op op,  int root,  MPI_Comm comm)
{
    MPI_Request request;
    MPI_Ireduce(sendbuf, recvbuf, count, datatype, op, root, comm, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}
#endif

int MPI_Ireduce_scatter(const void  *sendbuf,
                        void        *recvbuf,
                        const int    recvcounts[],
                        MPI_Datatype datatype,
                        MPI_Op       op,
                        MPI_Comm     comm,
                        MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{std::function{[=](MPI_Request *request_) {
                                          return PMPI_Ireduce_scatter(
                                              sendbuf, recvbuf, recvcounts, datatype, op, comm, request_);
                                      }},
                                      request});
    return MPI_SUCCESS;
}

int MPI_Reduce_scatter(
    const void *sendbuf, void *recvbuf, const int recvcounts[], MPI_Datatype datatype, MPI_Op op, MPI_Comm comm)
{
    MPI_Request request;
    MPI_Ireduce_scatter(sendbuf, recvbuf, recvcounts, datatype, op, comm, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}

int MPI_Ireduce_scatter_block(const void  *sendbuf,
                              void        *recvbuf,
                              int          recvcount,
                              MPI_Datatype datatype,
                              MPI_Op       op,
                              MPI_Comm     comm,
                              MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{std::function{[=](MPI_Request *request_) {
                                          return PMPI_Ireduce_scatter_block(
                                              sendbuf, recvbuf, recvcount, datatype, op, comm, request_);
                                      }},
                                      request});
    return MPI_SUCCESS;
}

int MPI_Reduce_scatter_block(
    const void *sendbuf, void *recvbuf, int recvcount, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm)
{
    MPI_Request request;
    MPI_Ireduce_scatter_block(sendbuf, recvbuf, recvcount, datatype, op, comm, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}

int MPI_Iscan(const void  *sendbuf,
              void        *recvbuf,
              int          count,
              MPI_Datatype datatype,
              MPI_Op       op,
              MPI_Comm     comm,
              MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{std::function{[=](MPI_Request *request_) {
                                          return PMPI_Iscan(sendbuf, recvbuf, count, datatype, op, comm, request_);
                                      }},
                                      request});
    return MPI_SUCCESS;
}

int MPI_Scan(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm)
{
    MPI_Request request;
    MPI_Iscan(sendbuf, recvbuf, count, datatype, op, comm, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}

int MPI_Iscatter(const void  *sendbuf,
                 int          sendcount,
                 MPI_Datatype sendtype,
                 void        *recvbuf,
                 int          recvcount,
                 MPI_Datatype recvtype,
                 int          root,
                 MPI_Comm     comm,
                 MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{
        std::function{[=](MPI_Request *request_) {
            return PMPI_Iscatter(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm, request_);
        }},
        request});
    return MPI_SUCCESS;
}

int MPI_Scatter(const void  *sendbuf,
                int          sendcount,
                MPI_Datatype sendtype,
                void        *recvbuf,
                int          recvcount,
                MPI_Datatype recvtype,
                int          root,
                MPI_Comm     comm)
{
    MPI_Request request;
    MPI_Iscatter(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}

int MPI_Iscatterv(const void  *sendbuf,
                  const int    sendcounts[],
                  const int    displs[],
                  MPI_Datatype sendtype,
                  void        *recvbuf,
                  int          recvcount,
                  MPI_Datatype recvtype,
                  int          root,
                  MPI_Comm     comm,
                  MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{
        std::function{[=](MPI_Request *request_) {
            return PMPI_Iscatterv(
                sendbuf, sendcounts, displs, sendtype, recvbuf, recvcount, recvtype, root, comm, request_);
        }},
        request});
    return MPI_SUCCESS;
}

int MPI_Scatterv(const void  *sendbuf,
                 const int    sendcounts[],
                 const int    displs[],
                 MPI_Datatype sendtype,
                 void        *recvbuf,
                 int          recvcount,
                 MPI_Datatype recvtype,
                 int          root,
                 MPI_Comm     comm)
{
    MPI_Request request;
    MPI_Iscatterv(sendbuf, sendcounts, displs, sendtype, recvbuf, recvcount, recvtype, root, comm, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}

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

#if 0
int MPI_File_iread_at(MPI_File fh, MPI_Offset offset, void *buf,
                                     int count, MPI_Datatype datatype, MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{std::function{[=](MPI_Request *request_) {
                                   return PMPI_File_iread_at(
                                       fh, offset, buf, count, datatype, request_);
                               }},
                                      request});
    return MPI_SUCCESS;
}

int MPI_File_read_at(MPI_File fh,  MPI_Offset offset,  void *buf, 
                                     int count,  MPI_Datatype datatype)
{
    MPI_Request request;
    MPI_File_iread_at(fh, offset, buf, count, datatype, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}


int MPI_File_iwrite_at(MPI_File fh, MPI_Offset offset, const void *buf,
                                      int count, MPI_Datatype datatype, MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{std::function{[=](MPI_Request *request_) {
                                   return PMPI_File_iwrite_at(
                                       fh, offset, buf, count, datatype, request_);
                               }},
                                      request});
    return MPI_SUCCESS;
}

int MPI_File_write_at(MPI_File fh,  MPI_Offset offset,  const void *buf, 
                                      int count,  MPI_Datatype datatype)
{
    MPI_Request request;
    MPI_File_iwrite_at(fh, offset, buf, count, datatype, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}


int MPI_File_iread_at_all(MPI_File fh, MPI_Offset offset, void *buf,
                                     int count, MPI_Datatype datatype, MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{std::function{[=](MPI_Request *request_) {
                                   return PMPI_File_iread_at_all(
                                       fh, offset, buf, count, datatype, request_);
                               }},
                                      request});
    return MPI_SUCCESS;
}

int MPI_File_read_at_all(MPI_File fh,  MPI_Offset offset,  void *buf, 
                                     int count,  MPI_Datatype datatype)
{
    MPI_Request request;
    MPI_File_iread_at_all(fh, offset, buf, count, datatype, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}


int MPI_File_iwrite_at_all(MPI_File fh, MPI_Offset offset, const void *buf,
                                      int count, MPI_Datatype datatype, MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{std::function{[=](MPI_Request *request_) {
                                   return PMPI_File_iwrite_at_all(
                                       fh, offset, buf, count, datatype, request_);
                               }},
                                      request});
    return MPI_SUCCESS;
}

int MPI_File_write_at_all(MPI_File fh,  MPI_Offset offset,  const void *buf, 
                                      int count,  MPI_Datatype datatype)
{
    MPI_Request request;
    MPI_File_iwrite_at_all(fh, offset, buf, count, datatype, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}


int MPI_File_iread(MPI_File fh, void *buf, int count,
                                  MPI_Datatype datatype, MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{std::function{[=](MPI_Request *request_) {
                                   return PMPI_File_iread(
                                       fh, buf, count, datatype, request_);
                               }},
                                      request});
    return MPI_SUCCESS;
}

int MPI_File_read(MPI_File fh,  void *buf,  int count, 
                                  MPI_Datatype datatype)
{
    MPI_Request request;
    MPI_File_iread(fh, buf, count, datatype, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}


int MPI_File_iwrite(MPI_File fh, const void *buf, int count,
                                   MPI_Datatype datatype, MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{std::function{[=](MPI_Request *request_) {
                                   return PMPI_File_iwrite(
                                       fh, buf, count, datatype, request_);
                               }},
                                      request});
    return MPI_SUCCESS;
}

int MPI_File_write(MPI_File fh,  const void *buf,  int count, 
                                   MPI_Datatype datatype)
{
    MPI_Request request;
    MPI_File_iwrite(fh, buf, count, datatype, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}

int MPI_File_iread_all(MPI_File fh, void *buf, int count,
                                  MPI_Datatype datatype, MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{std::function{[=](MPI_Request *request_) {
                                   return PMPI_File_iread_all(
                                       fh, buf, count, datatype, request_);
                               }},
                                      request});
    return MPI_SUCCESS;
}

int MPI_File_read_all(MPI_File fh,  void *buf,  int count, 
                                  MPI_Datatype datatype)
{
    MPI_Request request;
    MPI_File_iread_all(fh, buf, count, datatype, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}


int MPI_File_iwrite_all(MPI_File fh, const void *buf, int count,
                                   MPI_Datatype datatype, MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{std::function{[=](MPI_Request *request_) {
                                   return PMPI_File_iwrite_all(
                                       fh, buf, count, datatype, request_);
                               }},
                                      request});
    return MPI_SUCCESS;
}

int MPI_File_write_all(MPI_File fh,  const void *buf,  int count, 
                                   MPI_Datatype datatype)
{
    MPI_Request request;
    MPI_File_iwrite_all(fh, buf, count, datatype, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}


int MPI_File_iread_shared(MPI_File fh, void *buf, int count,
                                         MPI_Datatype datatype, MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{std::function{[=](MPI_Request *request_) {
                                   return PMPI_File_iread_shared(
                                       fh, buf, count, datatype, request_);
                               }},
                                      request});
    return MPI_SUCCESS;
}

int MPI_File_read_shared(MPI_File fh,  void *buf,  int count, 
                                         MPI_Datatype datatype)
{
    MPI_Request request;
    MPI_File_iread_shared(fh, buf, count, datatype, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}


int MPI_File_iwrite_shared(MPI_File fh, const void *buf, int count,
                                          MPI_Datatype datatype, MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{std::function{[=](MPI_Request *request_) {
                                   return PMPI_File_iwrite_shared(
                                       fh, buf, count, datatype, request_);
                               }},
                                      request});
    return MPI_SUCCESS;
}

int MPI_File_write_shared(MPI_File fh,  const void *buf,  int count, 
                                          MPI_Datatype datatype)
{
    MPI_Request request;
    MPI_File_iwrite_shared(fh, buf, count, datatype, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}
#endif

} /* extern "C" */

