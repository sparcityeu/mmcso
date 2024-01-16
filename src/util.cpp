#include <iostream>

#ifndef _GNU_SOURCE
#    define _GNU_SOURCE
#endif /* _GNU_SOURCE */

#include <climits>
#include <cstring>

#include <pthread.h>
#include <sched.h>
#include <unistd.h>

namespace mmcso::util
{
    void set_offload_thread_affinity(pthread_t thread_handle, int rank)
    {
        /** TODO: implement for multiple offloading threads */
        /** TODO: implement for affinity masks spanning multiple cpus */

        char *cpu_id_str = std::getenv("MMCSO_THREAD_AFFINITY");
        if (cpu_id_str == nullptr) {
            return;
        }

        bool  display_affinity     = false;
        char *display_affinity_str = std::getenv("MMCSO_DISPLAY_AFFINITY");
        if (display_affinity_str) {
            if (!strcasecmp(display_affinity_str, "true")) {
                display_affinity = true;
            }

            try {
                const int val{std::stoi(display_affinity_str)};
                if (val == 1) {
                    display_affinity = true;
                }
            } catch (std::invalid_argument const &ex) {
            } catch (std::out_of_range const &ex) {
            }
        }

        char hostname[HOST_NAME_MAX];
        gethostname(hostname, HOST_NAME_MAX);

        int r, cpu;
        int nscan;
        int offset = 0;
        int nread;
        // NOLINTNEXTLINE
        while ((nscan = sscanf(cpu_id_str + offset, "%d:%d%n[^,]", &r, &cpu, &nread)) == 2) {
            // std::cout << r << ":" << cpu << " nread: " << nread << '\n';
            offset += nread;
            if (cpu_id_str[offset] == ',') {
                ++offset;
            }
            if (r == rank) {
                cpu_set_t cpuset;
                CPU_ZERO(&cpuset);
                CPU_SET(cpu, &cpuset);
                int rc = pthread_setaffinity_np(thread_handle, sizeof(cpu_set_t), &cpuset);
                if (rc == 0 && display_affinity) {
                    std::cerr << "[libmmcso] : offloading thread affinity on [host " << hostname << ", rank " << r
                              << "] : cpu=" << cpu << '\n';
                }
                break;
            }
        }
    }
} // namespace mmcso::util