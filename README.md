# libmmc.so: A library for multithreaded MPI communication software offloading

This is an interposition library implementation of the software offloading concept [1] for MPI operations.

## Compilation and Installation

```bash
./build.sh
```

## Usage

Using the library requires preloading or linking *libmmc.so* to the application binary.

```bash
# Preload libmmc.so
export LD_PRELOAD={PATH TO LIBMMCSO}
# Run MPI application
```

### Thread affinity

It is recommended to pin the offloading thread to a core in the same NUMA domain as the threads of the multithreaded MPI process.
Controlling thread affinity with *libmmc.so* is possible using the environment variables `MMCSO_THREAD_AFFINITY` and `MMCSO_DISPLAY_AFFINITY`.

Syntax:

```bash
# Set thread affinity of offloading thread(s)
MMCSO_THREAD_AFFINITY={LIST}
LIST=rank:cpu[,LIST]
# Show thread affinity of offloading thread
MMCSO_DISPLAY_AFFINITY={TRUE|FALSE}
```

See `tools/run.sh` for an example with OpenMPI with 2 MPI processes and 3 OpenMP threads per MPI process.

## References

[1] 2015 Vaidyanathan et al.: [Improving concurrency and asynchrony in multithreaded MPI applications using software offloading](https://pavanbalaji.github.io/pubs/2015/sc/sc15.async_mpi.pdf)
