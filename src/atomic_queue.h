#ifndef MMCSO_ATOMIC_QUEUE_H_INCLUDED
#define MMCSO_ATOMIC_QUEUE_H_INCLUDED

#include "atomic_queue/atomic_queue.h"
#include "offload.h"

namespace mmcso
{
    using Element = OffloadCommand *;

    template <size_t QueueSize>
    class AtomicQueue
    {
    public:
        void    enqueue(Element elem) { q_.push(elem); }
        Element dequeue()
        {
            Element value;
            if (q_.try_pop(value)) {
                return value;
            }
            return nullptr;
        }

    private:
        atomic_queue::AtomicQueue<Element, QueueSize> q_;
    };
} // namespace mmcso

#endif /* MMCSO_ATOMIC_QUEUE_H_INCLUDED */
