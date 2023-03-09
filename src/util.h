#ifndef MMCSO_UTIL_H_INCLUDED
#define MMCSO_UTIL_H_INCLUDED

#include <pthread.h>

namespace mmcso::util
{
    void set_offload_thread_affinity(pthread_t thread_handle, int rank);
} // namespace mmcso::util

#endif /* MMCSO_UTIL_H_INCLUDED */