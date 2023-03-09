#ifndef MMCSO_OFFLOAD_H_INCLUDED
#define MMCSO_OFFLOAD_H_INCLUDED

#include <functional>
#include <thread>

#include <mpi.h>

#include "util.h"

namespace mmcso
{
    struct OffloadCommand {
        using MPICommand = std::function<int(MPI_Request *)>;
        explicit OffloadCommand(MPICommand &&func, MPI_Request *request) : func_{func}, request_{request} {}

        int operator()(MPI_Request *request) const { return func_(request); }

        MPICommand   func_;
        MPI_Request *request_;
    };

    template <class CommandQueue, class RequestManager, size_t NumThreads>
    class OffloadThread
    {
    public:
        explicit OffloadThread(CommandQueue &q, RequestManager &rm) : q_{q}, rm_{rm} {}

        void poll_and_test()
        {
            while (running_) {
                OffloadCommand *command = q_.dequeue();

                if (command) {
                    MPI_Request *request = rm_.post(command->request_);

                    int ret = (*command)(request);

                    delete command;

                    rm_.test_request(request);

                } else {
                    rm_.test_requests();
                }
            }
        }

        void run(int *argc, char ***argv, std::promise<int> provided_promise)
        {
            if constexpr (NumThreads == 1) {
                int provided;
                PMPI_Init_thread(argc, argv, MPI_THREAD_FUNNELED, &provided);
                provided_promise.set_value(provided);
            }
            int rank;
            MPI_Comm_rank(MPI_COMM_WORLD, &rank);

            util::set_offload_thread_affinity(thread_.native_handle(), rank);
            poll_and_test();
        }

        void start(int *argc, char ***argv, std::promise<int> provided_promise)
        {
            running_ = true;
            thread_  = std::thread{&OffloadThread::run, this, argc, argv, std::move(provided_promise)};
        }

        void stop()
        {
            running_ = false;
            thread_.join();
        }

        void enqueue(OffloadCommand *command) { q_.enqueue(command); }

    private:
        std::thread     thread_;
        CommandQueue   &q_;
        RequestManager &rm_;
        bool            running_{false};
    };

    template <class CommandQueue, class RequestManager, size_t NumThreads = 1>
    class OffloadEngine
    {
        static_assert(NumThreads > 0);

    public:
        void post(OffloadCommand *command)
        {
            // the application thread must eventually invalidate its request because it has to wait until
            // the offloading thread dequeues the command and provides a valid request
            rm_.invalidate_request(command->request_);
            ot_.enqueue(command);
        }

        void wait(MPI_Request *request, MPI_Status *status) { rm_.wait(request, status); }

        bool test(MPI_Request *request, MPI_Status *status) { return rm_.test(request, status); }

        void start(int *argc, char ***argv, std::promise<int> provided_promise)
        {
            if constexpr (NumThreads > 1) {
                int provided;
                PMPI_Init_thread(argc, argv, MPI_THREAD_MULTIPLE, &provided);
                provided_promise.set_value(provided);
            }
            ot_.start(argc, argv, std::move(provided_promise));
        }

        void stop() { ot_.stop(); }

        static OffloadEngine &instance()
        {
            static OffloadEngine oe{};
            return oe;
        }

        static constexpr size_t num_threads{NumThreads};

    private:
        CommandQueue                                            q_{};
        RequestManager                                          rm_{};
        OffloadThread<CommandQueue, RequestManager, NumThreads> ot_{q_, rm_};
    };
} // namespace mmcso

#endif /* MMCSO_OFFLOAD_H_INCLUDED */