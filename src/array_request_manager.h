#ifndef MMCSO_ARRAY_REQUEST_MANAGER_H_INCLUDED
#define MMCSO_ARRAY_REQUEST_MANAGER_H_INCLUDED

#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <limits>
#include <list>

#include <mpi.h>

#if NDEBUG
#    define DEBUG(fmt, ...)
#else
#    include <unistd.h>
#    define DEBUG(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__);
#endif /* NDEBUG */

namespace mmcso
{
    template <size_t PoolSize>
    class ArrayRequestManager
    {
        using index_t = uint32_t;
        // static_assert(std::atomic<bool>::is_always_lock_free);

        // required for using the MPI_Request as index:
        static_assert(sizeof(MPI_Request) >= sizeof(index_t));
        // required for reinterpret cast:
        static_assert(alignof(MPI_Request) >= alignof(index_t));

    public:
        void test_request(MPI_Request *request) { rc_.test_request(request); }

        void test_requests() { rc_.test_requests(); }

        MPI_Request *post(MPI_Request *request)
        {
            // if(!request) {
            // TODO: nullptr request not allowed according to MPI standard
            // possibly abort the MPI application here
            // }
            index_t idx = rc_.acquire();

            rc_.flags_[idx].done_ = false; // TODO: think about required memory ordering

            // return slot index in old request
            std::atomic<index_t> *idx_ptr = reinterpret_cast<std::atomic<index_t> *>(request);
            *idx_ptr                      = idx; // TODO: think about required memory ordering

            // new MPI_Request pointer
            return &rc_.requests_[idx];
        }

        /**
         * @brief Invalidates a request (inside application) to be able to test/wait on the request in the application
         * thread before the offload thread has posted the related offload command (called by application thread)
         *
         * @param request
         */
        void invalidate_request(MPI_Request *request)
        {
            std::atomic<index_t> *idx_ptr = reinterpret_cast<std::atomic<index_t> *>(request);
            *idx_ptr                      = INVALID_INDEX; // TODO: think about required memory ordering
        }

        /**
         * @brief Waits on a request (called by application thread)
         *
         * @param request
         * @param status
         */
        void wait(MPI_Request *request, MPI_Status *status)
        {
            index_t idx;
            // need to wait until offloading thread has set the index
            do {
                idx = *(reinterpret_cast<std::atomic<index_t> *>(request));
            } while (idx == INVALID_INDEX);

            while (!rc_.flags_[idx].done_) {
                /* spin */
            }

            if (status != MPI_STATUS_IGNORE) {
                *status = rc_.statuses_[idx];
            }

            rc_.flags_[idx].done_ = false;
            rc_.release(idx);
        }

        /**
         * @brief Tests completion of a request (called by application thread)
         *
         * @param request
         * @param status
         * @return true
         * @return false
         */
        bool test(MPI_Request *request, MPI_Status *status)
        {
            index_t idx = *(reinterpret_cast<std::atomic<index_t> *>(request));

            if (idx == INVALID_INDEX) {
                return false;
            }

            if (!rc_.flags_[idx].done_) {
                return false;
            }

            if (status != MPI_STATUS_IGNORE) {
                *status = rc_.statuses_[idx];
            }
            return true;
        }

    private:
        // we align the request flags to the cache line size to avoid false
        // sharing while spinning on the done flag
        //
        // TODO: this struct may may be merged with e.g. the MPI request or status
        // to use less memory and further improve performance
        struct alignas(CLSIZE) RequestFlags {
            std::atomic_bool done_{false};
            // char padding__[CLSIZE - sizeof(std::atomic_bool)];
        };

        template <size_t Size>
        struct RequestPool {
            RequestPool()
            {
                // not required to initialize requests
                // std::fill(requests_.begin(), requests_.end(), MPI_REQUEST_NULL);

                // this initializes the free list for the request pool
                for (index_t i = Size - 1; i != 0u; --i) {
                    release(i);
                }
            }

            void release(index_t idx)
            {
                nexts_[idx] = head_;
                while (!head_.compare_exchange_weak(
                    nexts_[idx], idx, std::memory_order_release, std::memory_order_relaxed)) {
                    /* spin */
                }
            }

            index_t acquire()
            {
                index_t old_head = head_;

                while (old_head != INVALID_INDEX &&
                       !head_.compare_exchange_weak(
                           old_head, nexts_[old_head], std::memory_order_acquire, std::memory_order_relaxed)) {
                    /* spin */
                }

                if (old_head == INVALID_INDEX) {
                    // TODO:
                    // this means that all request slots are in use
                    // this situation can lead to a deadlock situation
                    // solution can be to increase the number of available
                    // slots dynamically here
                    DEBUG("[pid=%d] no more available slots!\n", (int)getpid());
                    // test_requests();
                    // acquire();
                }

                used_.push_front(old_head);

                return old_head;
            }

            void test_request(MPI_Request *request)
            {
                index_t idx = request - &requests_[0];

                int flag;
                PMPI_Test(request, &flag, &statuses_[idx]);
                if (flag) {
                    used_.remove(idx);
                    flags_[idx].done_ = true;
                }
            }

            void test_requests()
            {
                // not using for_each since we need the iterator in for 'erase'
                for (auto it = used_.begin(); it != used_.end(); ++it) {
                    index_t idx = *it;
                    if (requests_[idx] == MPI_REQUEST_NULL) {
                        continue;
                    }
                    int flag;
                    PMPI_Test(&requests_[idx], &flag, &statuses_[idx]);
                    if (flag) {
                        flags_[idx].done_ = true;
                        it                = used_.erase(it);
                    }
                }
            }

            void reset(index_t idx) { flags_[idx].done_.store(false); }

            std::array<MPI_Request, Size>  requests_{};
            std::array<MPI_Status, Size>   statuses_{}; // holds MPI_Statuses of requests
            std::array<RequestFlags, Size> flags_{};
            std::array<index_t, Size>      nexts_{}; // stores index of slot's next free slot

            static constexpr index_t size_max_{Size}; // maximum number of outstanding MPI requests

            index_t size_{0u};  // current number of elements in use
            index_t last_{0u};  // index of next free element
            index_t first_{0u}; // index of first used element

            std::atomic<index_t> head_{INVALID_INDEX}; // head of (concurrent) free list
            std::list<index_t>   used_{}; // list of slots currently in use (managed only by offloading thread)
        };

        static constexpr index_t INVALID_INDEX = std::numeric_limits<index_t>::max();
        RequestPool<PoolSize>    rc_{};
    };
} // namespace mmcso

#endif /* MMCSO_ARRAY_REQUEST_MANAGER_H_INCLUDED */
