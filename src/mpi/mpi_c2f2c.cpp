#include <mpi.h>

#include "mpi_interposition.h"

extern "C" {

/***********************************************
 * MPI C and Fortran conversions
 ***********************************************/

#if MMCSO_OFFLOAD_NOT_IMPLEMENTED_YET

MPI_Fint       MPI_Errhandler_c2f(MPI_Errhandler errhandler);
MPI_Errhandler MPI_Errhandler_f2c(MPI_Fint errhandler);

MPI_Fint    MPI_Message_c2f(MPI_Message message);
MPI_Message MPI_Message_f2c(MPI_Fint message);

MPI_Fint    MPI_Op_c2f(MPI_Op op);
MPI_Op      MPI_Op_f2c(MPI_Fint op);

MPI_Fint    MPI_Request_c2f(MPI_Request request);
MPI_Request MPI_Request_f2c(MPI_Fint request);

int MPI_Status_c2f(const MPI_Status *c_status, MPI_Fint *f_status);
int MPI_Status_f2c(const MPI_Fint *f_status, MPI_Status *c_status);

MPI_Fint MPI_File_c2f(MPI_File file);
MPI_File MPI_File_f2c(MPI_Fint file);

MPI_Fint       MPI_Comm_c2f(MPI_Comm comm);
MPI_Comm       MPI_Comm_f2c(MPI_Fint comm);

MPI_Fint  MPI_Group_c2f(MPI_Group group);
MPI_Group MPI_Group_f2c(MPI_Fint group);

MPI_Fint MPI_Info_c2f(MPI_Info info);
MPI_Info MPI_Info_f2c(MPI_Fint info);

MPI_Fint MPI_Type_c2f(MPI_Datatype datatype);
MPI_Datatype MPI_Type_f2c(MPI_Fint datatype);

MPI_Fint MPI_Win_c2f(MPI_Win win);
MPI_Win  MPI_Win_f2c(MPI_Fint win);

#endif /* MMCSO_OFFLOAD_NOT_IMPLEMENTED_YET */

} /* extern "C" */
