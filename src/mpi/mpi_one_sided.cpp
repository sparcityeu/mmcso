#include <mpi.h>

#include "mpi_interposition.h"

extern "C" {

/***********************************************
 * MPI one-sided
 ***********************************************/

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
    oe.post(new mmcso::OffloadCommand{std::function{[=]([[maybe_unused]] MPI_Request *request_) {
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

#if MMCSO_OFFLOAD_NOT_IMPLEMENTED_YET

int            MPI_Compare_and_swap(const void  *origin_addr,
                                    const void  *compare_addr,
                                    void        *result_addr,
                                    MPI_Datatype datatype,
                                    int          target_rank,
                                    MPI_Aint     target_disp,
                                    MPI_Win      win);

int MPI_Fetch_and_op(const void  *origin_addr,
                          void        *result_addr,
                          MPI_Datatype datatype,
                          int          target_rank,
                          MPI_Aint     target_disp,
                          MPI_Op       op,
                          MPI_Win      win);

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

int MPI_Put(const void  *origin_addr,
            int          origin_count,
            MPI_Datatype origin_datatype,
            int          target_rank,
            MPI_Aint     target_disp,
            int          target_count,
            MPI_Datatype target_datatype,
            MPI_Win      win);

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
#endif /* MMCSO_OFFLOAD_NOT_IMPLEMENTED_YET */

} /* extern "C" */