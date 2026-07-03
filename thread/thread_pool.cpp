#include "thread/thread_pool.h"

namespace dry {
namespace thread {

ThreadPool::ThreadPool(size_t num_threads) {
  m_workers.reserve(num_threads);
  for (size_t i = 0; i < num_threads; ++i) {
    m_workers.emplace_back([this] { Worker(); });
  }
}

ThreadPool::~ThreadPool() {
  if (!m_stop) {
    Stop();
  }
}

void ThreadPool::Stop() {
  m_stop.store(true);
  m_condition.notify_all();
  for (auto& Worker : m_workers) {
    if (Worker.joinable()) {
      Worker.join();
    }
  }
}

const ThreadPool::Stats& ThreadPool::GetStats() const { return m_stats; }

void ThreadPool::SetMaxWaitTimeMs(uint64_t max_wait_time_ms) {
  m_max_wait_time_ms = max_wait_time_ms;
}

uint64_t ThreadPool::GetMaxWaitTimeMs() const { return m_max_wait_time_ms; }

std::size_t ThreadPool::GetQueueSize() const {
  std::lock_guard<std::mutex> lock(m_queue_mutex);
  return m_tasks.size();
}

std::size_t ThreadPool::GetThreadCount() const { return m_workers.size(); }

void ThreadPool::Worker() {
  while (true) {
    TaskWrapper task;
    {
      std::unique_lock<std::mutex> lock(m_queue_mutex);
      m_condition.wait(lock,
                       [this] { return m_stop.load() || !m_tasks.empty(); });
      if (m_stop.load() && m_tasks.empty()) {
        return;
      }
      task = std::move(m_tasks.front());
      m_tasks.pop();
    }
    // 计算排队等待时间
    auto wait_time = std::chrono::duration_cast<std::chrono::microseconds>(
                         TaskWrapper::Clock::now() - task.enqueue_time)
                         .count();
    // 更新统计信息
    m_stats.total_wait_time_us += wait_time;
    m_stats.wait_time_count++;
    // 超时丢弃检查
    if (m_max_wait_time_ms > 0 &&
        wait_time > m_max_wait_time_ms * 1000) {  // 转换为微秒
      m_stats.total_timeout_dropped++;
      continue;  // 丢弃任务
    }
    task.task();
    m_stats.total_completed++;
  }
}
}  // namespace thread
}  // namespace dry