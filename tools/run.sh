export LD_PRELOAD=${LD_PRELOAD}:build/lib/libmmcso.so

# this pins the offloading thread on rank 0 to cpu 7 and on rank 1 to cpu 15
export MMCSO_THREAD_AFFINITY=0:7,1:15
export MMCSO_DISPLAY_AFFINITY=TRUE

# OpenMPI + offloading using 2 ranks and 3 threads on cpu 0-5 and cpu 8-13
mpiexec -display-map -x OMP_DISPLAY_AFFINITY=true -x OMP_NUM_THREADS=3 -x OMP_PLACES=cores -n 2 --map-by node:PE=4 --bind-to core \
   time build/bench/mt_overlap -m 100000 -w 30000

# OpenMPI w/o offloading using 2 ranks and 3 threads on cpu 0-5 and cpu 8-13
export LD_PRELOAD=
mpiexec -display-map -x OMP_DISPLAY_AFFINITY=true -x OMP_NUM_THREADS=3 -x OMP_PLACES=cores -n 2 --map-by node:PE=4 --bind-to core \
    time build/bench/mt_overlap -m 100000 -w 30000
