#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <mpi.h>
#include <omp.h>

#ifndef _GNU_SOURCE
#    define _GNU_SOURCE
#endif /* _GNU_SOURCE */
#include <unistd.h>

struct alignas(CLSIZE) measurement {
    double post_time = 0.0;
    double wait_time = 0.0;
    double comp_time = 0.0;
    double comm_time = 0.0;
};

#define ARRAY_SIZE 1000000000

thread_local char *a{nullptr};
thread_local char *b{nullptr};

#define USE_MEMORY_INTENSIVE_WORK 0
#if USE_MEMORY_INTENSIVE_WORK
static void do_work(int work)
{
    work = 10 * work;

    static int i = 0;

    if (i + work > ARRAY_SIZE) {
        i = 0;
    }
    memcpy(a + i, b + i, work);
    i += work;
}
#else
static void do_work(int work)
{
    struct timespec ts {
        .tv_sec = 0, .tv_nsec = work
    };
    nanosleep(&ts, NULL);
}
#endif

int main(int argc, char *argv[])
{
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);

    int rank;
    int num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    const char *csv_file_default = "bench.csv";

    int   nthreads = omp_get_max_threads();
    int   work     = 0;
    int   msg_size = 1;
    int   rep      = 20000;
    char *csv_file = (char *)csv_file_default;

    MPI_Info info;
    MPI_Info_create(&info);
    MPI_Info_set(info, "mpi_assert_no_any_tag", "true");
    MPI_Info_set(info, "mpi_assert_no_any_source", "true");
    MPI_Info_set(info, "mpi_assert_exact_length", "true");
    MPI_Info_set(info, "mpi_assert_allow_overtaking", "true");
    MPI_Comm_set_info(MPI_COMM_WORLD, info);
    MPI_Info_free(&info);

    measurement m[nthreads];
    MPI_Comm    communicators[nthreads];

    for (int i = 0; i < nthreads; ++i) {
        if (MPI_Comm_dup(MPI_COMM_WORLD, &communicators[i]) != MPI_SUCCESS) {
            fprintf(stderr, "MPI_Comm_dup\n");
        }
    }

    int opt;
    while ((opt = getopt(argc, argv, "m:r:f:w:")) != -1) {
        switch (opt) {
        case 'r':
            rep = atoi(optarg);
            break;
        case 'm':
            msg_size = atoi(optarg);
            break;
        case 'f':
            csv_file = optarg;
            break;
        case 'w':
            work = atoi(optarg);
            break;
        default: /* '?' */
                 // usage:
            fprintf(stderr, "Usage: %s [-m message size] [-w work] [-r repetitions] [-f csv file]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    FILE *csv_out = fopen(csv_file, "a");
    if (!csv_out) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    // rank 0 <-> rank 1 <-> ... <-> rank N-1 <-> rank 0
    int rank_send = (rank + 1) % num_procs;
    int rank_recv = (rank + num_procs - 1) % num_procs;

#pragma omp parallel
    {
        int thread_id = omp_get_thread_num();

        // int *sendbuf = (int *)aligned_alloc(PAGESIZE, msg_size * sizeof(int));
        // int *recvbuf = (int *)aligned_alloc(PAGESIZE, msg_size * sizeof(int));

        size_t num_pages = ((msg_size * sizeof(int) + PAGESIZE - 1) / PAGESIZE) * PAGESIZE;
        assert(num_pages % PAGESIZE == 0);
        int *sendbuf = (int *)aligned_alloc(PAGESIZE, num_pages);
        int *recvbuf = (int *)aligned_alloc(PAGESIZE, num_pages);
        if (!sendbuf || !recvbuf) {
            perror("aligned_alloc");
            exit(EXIT_FAILURE);
        }

        num_pages = ((ARRAY_SIZE + PAGESIZE - 1) / PAGESIZE) * PAGESIZE;
        assert(num_pages % PAGESIZE == 0);
        a = (char *)aligned_alloc(PAGESIZE, num_pages);
        b = (char *)aligned_alloc(PAGESIZE, num_pages);
        if (!a || !b) {
            perror("aligned_alloc");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < msg_size; ++i) {
            sendbuf[i] = thread_id + i;
        }

#pragma omp barrier

        for (int i = 0; i < rep; ++i) {

            MPI_Status  status_s, status_r;
            MPI_Request request_s, request_r;

            double time_start = omp_get_wtime();
            double total_time = time_start;
            double time_end;

            MPI_Irecv(recvbuf, msg_size, MPI_INT, rank_recv, thread_id, communicators[thread_id], &request_r);
            MPI_Isend(sendbuf, msg_size, MPI_INT, rank_send, thread_id, communicators[thread_id], &request_s);

            time_end = omp_get_wtime();

            m[thread_id].post_time += time_end - time_start;

            time_start = time_end;
            if (work) {
                do_work(work);
            }
            time_end = omp_get_wtime();

            m[thread_id].comp_time += time_end - time_start;

            time_start = time_end;

            MPI_Wait(&request_s, &status_s);
            MPI_Wait(&request_r, &status_r);

            time_end = omp_get_wtime();

            m[thread_id].wait_time += time_end - time_start;
            m[thread_id].comm_time += time_end - total_time;

#if !defined(NDEBUG)
            assert(status_s.MPI_SOURCE == rank);
            assert(status_s.MPI_TAG == thread_id);
            int count;
            MPI_Get_count(&status_r, MPI_INT, &count);
            assert(status_r.MPI_SOURCE == rank_recv);
            assert(status_r.MPI_TAG == thread_id);
            assert(count == msg_size);
            for (int i = 0; i < msg_size; ++i) {
                assert(recvbuf[i] == thread_id + i);
                recvbuf[i] = -1;
            }
#endif
        }
        free(sendbuf);
        free(recvbuf);

        free(a);
        free(b);
    } // parallel region end

    // MPI_Barrier(MPI_COMM_WORLD);

    double post_time = 0.0;
    double wait_time = 0.0;
    double comp_time = 0.0;
    double comm_time = 0.0;

    for (int i = 0; i < nthreads; ++i) {
        post_time += m[i].post_time;
        wait_time += m[i].wait_time;
        comp_time += m[i].comp_time;
        comm_time += m[i].comm_time;
    }

    printf("[rank %d] post time (avg. per thread and iter): %lfus\n", rank, 1.0e6 * post_time / nthreads / rep);
    printf("[rank %d] wait time (avg. per thread and iter): %lfus\n", rank, 1.0e6 * wait_time / nthreads / rep);
    printf("[rank %d] comp time (avg. per thread and iter): %lfus\n", rank, 1.0e6 * comp_time / nthreads / rep);
    printf("[rank %d] comm time (avg. per thread and iter): %lfus\n", rank, 1.0e6 * comm_time / nthreads / rep);
    printf("[rank %d] overlap: %.2lf%%\n", rank, 100. * comp_time / comm_time);
    fflush(stdout);

    for (int i = 0; i < nthreads; ++i) {
        fprintf(csv_out,
                "%d,%d,%d,%d,%zu,%lf,%lf,%lf,%lf\n",
                nthreads,
                i,
                rep,
                work,
                msg_size * sizeof(int),
                m[i].post_time,
                m[i].wait_time,
                m[i].comp_time,
                m[i].comm_time);
    }
    fflush(csv_out);
    fclose(csv_out);

    MPI_Finalize();
    return 0;
}
