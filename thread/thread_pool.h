#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

namespace dry {
namespace thread {

class ThreadPool {
 public:
  // 线程池统计信息
  struct Stats {
    std::atomic<uint64_t> total_submitted{0};        // 总提交任务数
    std::atomic<uint64_t> total_completed{0};        // 总完成任务数
    std::atomic<uint64_t> total_rejected{0};         // 总拒绝任务数
    std::atomic<uint64_t> total_timeout_dropped{0};  // 超时丢弃数

    // 用于计算平均排队等待时间
    std::atomic<uint64_t> total_wait_time_us{0};  // 累计排队等待时间(微秒)
    std::atomic<uint64_t> wait_time_count{0};  // 采样计数
  };

  // 任务包装器
  struct TaskWrapper {
    using Clock = std::chrono::steady_clock;
    std::function<void()> task;
    Clock::time_point enqueue_time;
    TaskWrapper() = default;
    explicit TaskWrapper(std::function<void()> task)
        : task(std::move(task)), enqueue_time(Clock::now()) {}
  };
  ThreadPool(size_t num_threads);
  ~ThreadPool();

  void stop();

  const Stats& getStats() const;
  void setMaxWaitTimeMs(uint64_t max_wait_time_ms);
  uint64_t getMaxWaitTimeMs() const;
  std::size_t getQueueSize() const;
  std::size_t getThreadCount() const;

  template <typename Func, typename... Args>
  auto submit(Func&& func, Args&&... args);

 private:
  void worker();

  std::vector<std::thread> m_workers;
  std::queue<TaskWrapper> m_tasks;

  mutable std::mutex m_queue_mutex;
  std::condition_variable m_condition;
  std::atomic_bool m_stop{false};
  Stats m_stats;
  uint64_t m_max_wait_time_ms{0};  // 最大排队等待时间(毫秒)
};

}  // namespace thread
}  // namespace dry

#include "thread_pool.inc"