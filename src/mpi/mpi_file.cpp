#include <mpi.h>

#include "mpi_interposition.h"

extern "C" {

/***********************************************
 * MPI file
 ***********************************************/

#if MMCSO_OFFLOAD_NOT_IMPLEMENTED_YET

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

#endif /* MMCSO_OFFLOAD_NOT_IMPLEMENTED_YET */

#if MMCSO_OFFLOAD_NOT_IMPLEMENTED_YET

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

#endif /* MMCSO_OFFLOAD_NOT_IMPLEMENTED_YET */

} /* extern "C" */