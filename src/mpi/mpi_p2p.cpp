#include <mpi.h>

#include "mpi_interposition.h"

extern "C" {

/***********************************************
 * MPI P2P
 ***********************************************/

int MPI_Irecv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Request *request)
{
    oe.post(new mmcso::OffloadCommand{std::function{[=](MPI_Request *request_) {
                                          return PMPI_Irecv(buf, count, datatype, source, tag, comm, request_);
                                      }},
                                      request});
    return MPI_SUCCESS;
}

int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status)
{
    MPI_Request request;
    MPI_Irecv(buf, count, datatype, source, tag, comm, &request);
    MPI_Wait(&request, status);
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

#if MMCSO_OFFLOAD_NOT_IMPLEMENTED_YET
int MPI_Mrecv(void *buf,  int count,  MPI_Datatype type, MPI_Message *message, MPI_Status *status)
{
    MPI_Request request;
    MPI_Imrecv(buf, count, type, message, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}
#endif /* MMCSO_OFFLOAD_NOT_IMPLEMENTED_YET */

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

#if MMCSO_OFFLOAD_NOT_IMPLEMENTED_YET

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

#endif /* MMCSO_OFFLOAD_NOT_IMPLEMENTED_YET */

} /* extern "C" */
