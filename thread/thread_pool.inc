#pragma once
#include <future>

#include "thread/thread_pool.h"

namespace dry {
namespace thread {
template <typename Func, typename... Args>
auto ThreadPool::Submit(Func&& func, Args&&... args) {
  using ReturnType = std::invoke_result_t<Func, Args...>;

  if (m_stop.load(std::memory_order_relaxed)) {
    m_stats.total_rejected.fetch_add(1, std::memory_order_relaxed);
    return std::future<ReturnType>();
  }

  auto task = std::make_shared<std::packaged_task<ReturnType()>>(
      std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
  auto wrapper = TaskWrapper([task]() { (*task)(); });

  {
    std::lock_guard<std::mutex> lock(m_queue_mutex);
    if (m_stop.load()) {
      m_stats.total_rejected.fetch_add(1, std::memory_order_relaxed);
      return std::future<ReturnType>();
    }
    m_tasks.emplace(std::move(wrapper));
    m_stats.total_submitted.fetch_add(1, std::memory_order_relaxed);
  }
  m_condition.notify_one();
  return task->get_future();
}
}  // namespace thread
}  // namespace dry