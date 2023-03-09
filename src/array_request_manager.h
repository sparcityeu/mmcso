#ifndef MMCSO_ARRAY_REQUEST_MANAGER_H_INCLUDED
#define MMCSO_ARRAY_REQUEST_MANAGER_H_INCLUDED

#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <limits>

#include <mpi.h>

namespace mmcso
{
    template <size_t PoolSize>
    class ArrayRequestManager
    {
        using index_t = uint32_t;
        static_assert(std::atomic<bool>::is_always_lock_free);
        static_assert(PoolSize > 0);
        // required for using the MPI_Request as index:
        static_assert(sizeof(MPI_Request) >= sizeof(index_t));
        // required for reinterpret cast:
        static_assert(alignof(MPI_Request) >= alignof(index_t));

    public:
        void test_request(MPI_Request *request) { rc_.test_request(request); }

        void test_requests()
        {
            rc_.test_requests();
            rc_.reclaim();
        }

        MPI_Request *post(MPI_Request *request)
        {
            index_t idx = rc_.next_slot_idx();

            // clear request in array
            rc_.requests_[idx]       = *request; // TODO: copy necessary?
            rc_.flags_[idx].done_    = false;
            rc_.flags_[idx].reclaim_ = false;

            // return slot index in old request
            std::atomic<index_t> *idx_ptr = reinterpret_cast<std::atomic<index_t> *>(request);
            *idx_ptr                      = idx;

            // new MPI_Request pointer
            return &rc_.requests_[idx];
        }

        void invalidate_request(MPI_Request *request)
        {
            std::atomic<index_t> *idx_ptr = reinterpret_cast<std::atomic<index_t> *>(request);
            *idx_ptr                      = INVALID_INDEX;
        }

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

            rc_.flags_[idx].reclaim_ = true;
        }

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

            rc_.flags_[idx].reclaim_ = true;
            return true;
        }

    private:
        struct alignas(CLSIZE) RequestFlags {
            std::atomic_bool done_{false};
            // char padding__[CLSIZE - sizeof(std::atomic_bool)];
            std::atomic_bool reclaim_{false};
        };

        template <size_t Size>
        struct RequestPool {
            RequestPool() { std::fill(requests_.begin(), requests_.end(), MPI_REQUEST_NULL); }

            index_t next_slot_idx()
            {
                // wait until a slot is free
                while (size_ == size_max_) {
                    test_requests();
                    reclaim();
                }

                if (size_ != 0) {
                    ++last_;
                    last_ = last_ % size_max_;
                }

                ++size_;

                return last_;
            }

            void test_request(MPI_Request *request)
            {
                index_t idx = request - &requests_[0];
                int     flag;
                PMPI_Test(request, &flag, &statuses_[idx]);
                if (flag) {
                    flags_[idx].done_ = true;
                }
            }

            void test_and_set_flags(index_t first, index_t len)
            {
                int outcount;
                PMPI_Testsome(len, &requests_[first], &outcount, &indices_[first], &statuses_[first]);

                if (outcount == 0 || outcount == MPI_UNDEFINED) {
                    return;
                }

                for (int i = 0; i < outcount; ++i) {
                    flags_[indices_[i + first] + first].done_ = true;
                }
            }

            void test_requests()
            {
                /** TODO: benchmark and select one of the two solutions */
                test_requests_array();
            }

            void test_requests_array()
            {
                if (size_ == 0) {
                    return;
                }

                index_t imax = last_ < first_ ? size_max_ : last_ + 1;
                test_and_set_flags(first_, imax - first_);
                if (last_ < first_) {
                    test_and_set_flags(0, last_ + 1);
                }
            }

            void test_requests_single()
            {
                if (size_ == 0) {
                    return;
                }

                int imax = last_ < first_ ? size_max_ : last_ + 1;
                for (int i = first_; i < imax; ++i) {
                    if (requests_[i] == MPI_REQUEST_NULL) {
                        continue;
                    }
                    int flag;
                    PMPI_Test(&requests_[i], &flag, &statuses_[i]);
                    if (flag) {
                        flags_[i].done_ = true;
                    }
                }

                if (last_ < first_) {
                    for (int i = 0; i <= last_; ++i) {
                        if (requests_[i] == MPI_REQUEST_NULL) {
                            continue;
                        }
                        int flag;
                        PMPI_Test(&requests_[i], &flag, &statuses_[i]);
                        if (flag) {
                            flags_[i].done_ = true;
                        }
                    }
                }
            }

            void reset(index_t idx)
            {
                // requests_[idx]       = MPI_REQUEST_NULL; // not necessary
                flags_[idx].done_    = false;
                flags_[idx].reclaim_ = false;
            }

            void reclaim()
            {
                if (size_ == 0) {
                    return;
                }

                while (flags_[first_].reclaim_) {
                    reset(first_);
                    --size_;
                    if (size_ > 0) {
                        ++first_;
                        first_ = first_ % size_max_;
                    }
                }
            }

            index_t size_max_{Size};
            index_t size_{0u};  // current number of elements in use
            index_t last_{0u};  // index of next free element
            index_t first_{0u}; // index of first used element

            std::array<MPI_Request, Size>  requests_{};
            std::array<RequestFlags, Size> flags_{};

            std::array<int, Size>        indices_{};  // array required for calling MPI_Testsome/any/all
            std::array<MPI_Status, Size> statuses_{}; // array required for calling MPI_Testsome/any/all
        };

        static constexpr index_t INVALID_INDEX = std::numeric_limits<index_t>::max();
        RequestPool<PoolSize>    rc_{};
    };
} // namespace mmcso

#endif /* MMCSO_ARRAY_REQUEST_MANAGER_H_INCLUDED */
