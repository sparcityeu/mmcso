
#include <mpi.h>

#include "mpi_interposition.h"

extern "C" {

/***********************************************
 * MPI windows
 ***********************************************/

int MPI_Win_create(void *base, MPI_Aint size, int disp_unit, MPI_Info info, MPI_Comm comm, MPI_Win *win)
{
    MPI_Request request; // = MPI_REQUEST_NULL;
    oe.post(new mmcso::OffloadCommand{std::function{[=]([[maybe_unused]] MPI_Request *request_) {
                                          return PMPI_Win_create(base, size, disp_unit, info, comm, win);
                                      }},
                                      &request,
                                      true});
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}

int MPI_Win_fence(int assert, MPI_Win win)
{
    MPI_Request request; // = MPI_REQUEST_NULL;
    oe.post(new mmcso::OffloadCommand{
        std::function{[=]([[maybe_unused]] MPI_Request *request_) { return PMPI_Win_fence(assert, win); }}, &request, true});
    MPI_Wait(&request, MPI_STATUS_IGNORE); // NOLINT: suppress warning 'no matching nonblocking call'
    return MPI_SUCCESS;
}


#if MMCSO_OFFLOAD_NOT_IMPLEMENTED_YET

int MPI_Win_allocate(MPI_Aint size, int disp_unit, MPI_Info info, MPI_Comm comm, void *baseptr, MPI_Win *win);
int MPI_Win_allocate_shared(MPI_Aint size, int disp_unit, MPI_Info info, MPI_Comm comm, void *baseptr, MPI_Win *win);
int MPI_Win_attach(MPI_Win win, void *base, MPI_Aint size);

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

#endif /* MMCSO_OFFLOAD_NOT_IMPLEMENTED_YET */

} /* extern "C" */