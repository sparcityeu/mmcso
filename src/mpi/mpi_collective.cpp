#include <mpi.h>

#include "mpi_interposition.h"

extern "C" {

/***********************************************
 * MPI collectives
 ***********************************************/

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

/***********************************************
 * MPI neighborhood collectives
 ***********************************************/

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

} /* extern "C" */