# export LD_PRELOAD=/lib/x86_64-linux-gnu/libasan.so.6
export LD_PRELOAD=${LD_PRELOAD}:build/src/libmmcso.so
export MMCSO_THREAD_AFFINITY=0:7,1:15
export MMCSO_DISPLAY_AFFINITY=TRUE

# OpenMPI
mpiexec -display-map -x OMP_DISPLAY_AFFINITY=true -x OMP_NUM_THREADS=3 -x OMP_PLACES=cores -n 2 --map-by node:PE=4 --bind-to core \
    build/bench/mt_overlap -m 100000 -w 30000

export LD_PRELOAD=
mpiexec -display-map -x OMP_DISPLAY_AFFINITY=true -x OMP_NUM_THREADS=3 -x OMP_PLACES=cores -n 2 --map-by node:PE=4 --bind-to core \
    build/bench/mt_overlap -m 100000 -w 30000
