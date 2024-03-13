#include <mpi.h>

#include "mpi_interposition.h"

extern "C" {

/***********************************************
 * MPI topology
 ***********************************************/

#if MMCSO_OFFLOAD_NOT_IMPLEMENTED_YET

int MPI_Cart_coords(MPI_Comm comm, int rank, int maxdims, int coords[]);
int MPI_Cart_create(
    MPI_Comm old_comm, int ndims, const int dims[], const int periods[], int reorder, MPI_Comm *comm_cart);
int            MPI_Cart_get(MPI_Comm comm, int maxdims, int dims[], int periods[], int coords[]);
int            MPI_Cart_map(MPI_Comm comm, int ndims, const int dims[], const int periods[], int *newrank);
int            MPI_Cart_rank(MPI_Comm comm, const int coords[], int *rank);
int            MPI_Cart_shift(MPI_Comm comm, int direction, int disp, int *rank_source, int *rank_dest);
int            MPI_Cart_sub(MPI_Comm comm, const int remain_dims[], MPI_Comm *new_comm);
int            MPI_Cartdim_get(MPI_Comm comm, int *ndims);

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

int            MPI_Dims_create(int nnodes, int ndims, int dims[]);

int MPI_Graph_create(
    MPI_Comm comm_old, int nnodes, const int index[], const int edges[], int reorder, MPI_Comm *comm_graph);
int       MPI_Graph_get(MPI_Comm comm, int maxindex, int maxedges, int index[], int edges[]);
int       MPI_Graph_map(MPI_Comm comm, int nnodes, const int index[], const int edges[], int *newrank);
int       MPI_Graph_neighbors_count(MPI_Comm comm, int rank, int *nneighbors);
int       MPI_Graph_neighbors(MPI_Comm comm, int rank, int maxneighbors, int neighbors[]);
int       MPI_Graphdims_get(MPI_Comm comm, int *nnodes, int *nedges);


int      MPI_Topo_test(MPI_Comm comm, int *status);

#endif /* MMCSO_OFFLOAD_NOT_IMPLEMENTED_YET */

} /* extern "C" */
