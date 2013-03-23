#ifndef NUCLEAR_EXECUTIONCORE_H
#define NUCLEAR_EXECUTIONCORE_H

#include <thread>
#include <iostream>
#include <chrono>
#include "Reaction.h"
#include "BlockingQueue.h"

namespace NUClear {
    class ExecutionCore {
    public:
        ExecutionCore(BlockingQueue<std::unique_ptr<Reaction>>& queue);
        ExecutionCore(const ExecutionCore& other) = delete;
        ExecutionCore& operator=(const ExecutionCore& other) = delete;
        ExecutionCore(ExecutionCore&& other);
        ExecutionCore& operator=(ExecutionCore&& other);
        friend void swap(ExecutionCore& first, ExecutionCore& second);

        ~ExecutionCore();

        std::thread::id getThreadId();

        void kill();
        void join();
        reactionId_t getCurrentReactionId();
    private:
        void core();

        bool execute;
        reactionId_t currentReactionId;
        BlockingQueue<std::unique_ptr<Reaction>>& queue;
        std::thread thread;
    };
}

#endif
