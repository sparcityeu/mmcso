#include <functional>
#include <future>

#include <mpi.h>

#include "mpi_interposition.h"

/* static */ OE &oe = OE::instance();

extern "C" {

int MPI_Init_thread(int *argc, char ***argv, [[maybe_unused]] int required, int *provided)
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

#if REQUIRED
int MPI_Query_thread(int *provided);
#endif

/***********************************************
 * MPI wait / test
 ***********************************************/

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

#if MMCSO_OFFLOAD_NOT_IMPLEMENTED_YET
int MPI_Waitany(int count, MPI_Request array_of_requests[], int *index, MPI_Status *status);
int MPI_Waitsome(int         incount,
                 MPI_Request array_of_requests[],
                 int        *outcount,
                 int         array_of_indices[],
                 MPI_Status  array_of_statuses[]);
#endif

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
int MPI_Testall(int count, MPI_Request array_of_requests[], int *flag, MPI_Status array_of_statuses[]);
int MPI_Testany(int count, MPI_Request array_of_requests[], int *index, int *flag, MPI_Status *status);
int MPI_Test_cancelled(const MPI_Status *status, int *flag);
int MPI_Testsome(int         incount,
                 MPI_Request array_of_requests[],
                 int        *outcount,
                 int         array_of_indices[],
                 MPI_Status  array_of_statuses[]);
#endif /* MMCSO_OFFLOAD_NOT_IMPLEMENTED_YET */

#if MMCSO_OFFLOAD_NOT_IMPLEMENTED_YET

int MPI_Abort(MPI_Comm comm, int errorcode);

int MPI_Add_error_class(int *errorclass);
int MPI_Add_error_code(int errorclass, int *errorcode);
int MPI_Add_error_string(int errorcode, const char *string);

int MPI_Alloc_mem(MPI_Aint size, MPI_Info info, void *baseptr);

int MPI_Buffer_attach(void *buffer, int size);
int MPI_Buffer_detach(void *buffer, int *size);


int            MPI_Errhandler_free(MPI_Errhandler *errhandler);
int            MPI_Error_class(int errorcode, int *errorclass);
int            MPI_Error_string(int errorcode, char *string, int *resultlen);

int            MPI_Close_port(const char *port_name);

int MPI_Finalize(void);
int MPI_Finalized(int *flag);
int MPI_Free_mem(void *base);

int MPI_Get_address(const void *location, MPI_Aint *address);

int MPI_Get_library_version(char *version, int *resultlen);
int MPI_Get_processor_name(char *name, int *resultlen);
int MPI_Get_version(int *version, int *subversion);

int      MPI_Improbe(int source, int tag, MPI_Comm comm, int *flag, MPI_Message *message, MPI_Status *status);

int      MPI_Initialized(int *flag);

int      MPI_Intercomm_create(
         MPI_Comm local_comm, int local_leader, MPI_Comm bridge_comm, int remote_leader, int tag, MPI_Comm *newintercomm);
int MPI_Intercomm_merge(MPI_Comm intercomm, int high, MPI_Comm *newintercomm);

int         MPI_Is_thread_main(int *flag);
int         MPI_Lookup_name(const char *service_name, MPI_Info info, char *port_name);
int         MPI_Mprobe(int source, int tag, MPI_Comm comm, MPI_Message *message, MPI_Status *status);

int         MPI_Op_commutative(MPI_Op op, int *commute);
int         MPI_Op_create(MPI_User_function *function, int commute, MPI_Op *op);
int         MPI_Open_port(MPI_Info info, char *port_name);

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

int MPI_Reduce_local(const void *inbuf, void *inoutbuf, int count, MPI_Datatype datatype, MPI_Op op);

int         MPI_Register_datarep(const char                      *datarep,
                                 MPI_Datarep_conversion_function *read_conversion_fn,
                                 MPI_Datarep_conversion_function *write_conversion_fn,
                                 MPI_Datarep_extent_function     *dtype_file_extent_fn,
                                 void                            *extra_state);

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

double   MPI_Wtick(void);
double   MPI_Wtime(void);

#endif /* MMCSO_OFFLOAD_NOT_IMPLEMENTED_YET */

} /* extern "C" */

#include "mpi_fortran_wrappers.h"

#if 0

// MPI_Win_flush_all
int MPI_Blocking_call_wo_nonblocking_counterpart_and_wo_status(void *buf)
{
    MPI_Request request;
    oe.post(new mmcso::OffloadCommand{std::function{[=](MPI_Request *request_) {
                                          return MPI_Blocking_call_wo_nonblocking_counterpart_and_wo_status(buf);
                                      }},
                                      request});
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}


// MPI_Iprobe
int MPI_Nonblocking_wo_request_and_w_status(void *buf, MPI_Status *status)
{
    MPI_Request request;
    oe.post(new mmcso::OffloadCommand{std::function{[=](MPI_Request *request_) {
                                          return MPI_Nonblocking_wo_request_and_w_status(buf, status);
                                      }},
                                      request});
    MPI_Wait(&request, status);
    return MPI_SUCCESS;
}

// MPI_File_read_all, MPI_Recv, blocking call has status instead of request
int MPI_File_read_all(void *buf, MPI_Status *status)
{
    MPI_Request request;
    MPI_File_iread_all(..., &request);
    MPI_Wait(&request, status);
    return MPI_SUCCESS;
}

#endif
