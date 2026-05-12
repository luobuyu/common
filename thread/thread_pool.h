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

    Stats() = default;

    // 快照拷贝构造（用于 getStats() 返回值）
    Stats(const Stats& other)
        : total_submitted(
              other.total_submitted.load(std::memory_order_relaxed)),
          total_completed(
              other.total_completed.load(std::memory_order_relaxed)),
          total_rejected(other.total_rejected.load(std::memory_order_relaxed)),
          total_timeout_dropped(
              other.total_timeout_dropped.load(std::memory_order_relaxed)),
          total_wait_time_us(
              other.total_wait_time_us.load(std::memory_order_relaxed)),
          wait_time_count(
              other.wait_time_count.load(std::memory_order_relaxed)) {}

    Stats& operator=(const Stats& other) {
      if (this != &other) {
        total_submitted.store(
            other.total_submitted.load(std::memory_order_relaxed),
            std::memory_order_relaxed);
        total_completed.store(
            other.total_completed.load(std::memory_order_relaxed),
            std::memory_order_relaxed);
        total_rejected.store(
            other.total_rejected.load(std::memory_order_relaxed),
            std::memory_order_relaxed);
        total_timeout_dropped.store(
            other.total_timeout_dropped.load(std::memory_order_relaxed),
            std::memory_order_relaxed);
        total_wait_time_us.store(
            other.total_wait_time_us.load(std::memory_order_relaxed),
            std::memory_order_relaxed);
        wait_time_count.store(
            other.wait_time_count.load(std::memory_order_relaxed),
            std::memory_order_relaxed);
      }
      return *this;
    }

    /// 聚合另一个 Stats 的值（用于 ShardedThreadPool 汇总）
    /// @note 仅用于局部变量汇总，非线程安全
    Stats& operator+=(const Stats& other) {
      total_submitted.store(
          total_submitted.load(std::memory_order_relaxed) +
              other.total_submitted.load(std::memory_order_relaxed),
          std::memory_order_relaxed);
      total_completed.store(
          total_completed.load(std::memory_order_relaxed) +
              other.total_completed.load(std::memory_order_relaxed),
          std::memory_order_relaxed);
      total_rejected.store(
          total_rejected.load(std::memory_order_relaxed) +
              other.total_rejected.load(std::memory_order_relaxed),
          std::memory_order_relaxed);
      total_timeout_dropped.store(
          total_timeout_dropped.load(std::memory_order_relaxed) +
              other.total_timeout_dropped.load(std::memory_order_relaxed),
          std::memory_order_relaxed);
      total_wait_time_us.store(
          total_wait_time_us.load(std::memory_order_relaxed) +
              other.total_wait_time_us.load(std::memory_order_relaxed),
          std::memory_order_relaxed);
      wait_time_count.store(
          wait_time_count.load(std::memory_order_relaxed) +
              other.wait_time_count.load(std::memory_order_relaxed),
          std::memory_order_relaxed);
      return *this;
    }
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
  std::atomic<uint64_t> m_max_wait_time_ms{0};  // 最大排队等待时间(毫秒)
};

}  // namespace thread
}  // namespace dry

#include "thread_pool.inc"