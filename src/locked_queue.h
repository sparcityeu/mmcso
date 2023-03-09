#ifndef MMCSO_LOCKED_QUEUE_H_INCLUDED
#define MMCSO_LOCKED_QUEUE_H_INCLUDED

#include <mutex>
#include <queue>

#include "offload.h"

namespace mmcso
{
    using Element = OffloadCommand *;

    class LockedQueue
    {
    public:
        void    enqueue(Element);
        Element dequeue();

    private:
        std::queue<Element> q_;
        std::mutex          mtx_;
    };

    void LockedQueue::enqueue(Element elem)
    {
        std::lock_guard<std::mutex> lock{mtx_};
        q_.push(elem);
    }

    Element LockedQueue::dequeue()
    {
        Element value;

        std::lock_guard<std::mutex> lock{mtx_};
        if (q_.empty()) {
            return nullptr;
        }
        value = q_.front();
        q_.pop();

        return value;
    }
} // namespace mmcso

#endif /* MMCSO_LOCKED_QUEUE_H_INCLUDED */
