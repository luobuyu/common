#ifndef LOG_BLOCKING_QUEUE_H
#define LOG_BLOCKING_QUEUE_H
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <vector>

namespace logger {
    template <class T>
    class CircleQueue
    {
    public:
        CircleQueue() = default;
        explicit CircleQueue(int max_size);
        bool full() const;
        bool empty() const;
        std::size_t size() const;
        void push(T &&item);
        void push(const T &item);
        T &front();
        void popFront();
        void resize(int size);

    private:
        uint32_t m_max_size;
        uint32_t m_head;
        uint32_t m_tail;
        std::vector<T> m_vec;
    };

    template <class T>
    class BlockingQueue
    {
    public:
        BlockingQueue();
        explicit BlockingQueue(int size);
        ~BlockingQueue();
        void push(T &&item);
        void push(const T &item);
        bool tryPop(T &item);
        bool pop(T &item);

        int size() const;
        void resize(int size);
        bool empty();

        void stop();
        void waitStop();

    private:
        logger::CircleQueue<T> m_queue;
        mutable std::mutex m_mtx;
        std::condition_variable m_cond_producer;
        std::condition_variable m_cond_comsumer;
        std::atomic_bool m_is_stop_now;
        std::atomic_bool m_is_stop_wait;
    };
}

#include "blocking_queue.inc"
#endif