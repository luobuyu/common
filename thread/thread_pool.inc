#pragma once
#include <future>

#include "common/thread/thread_pool.h"

namespace dry {
namespace thread {
template <typename Func, typename... Args>
auto ThreadPool::submit(Func&& func, Args&&... args) {
  using ReturnType = std::invoke_result_t<Func, Args...>;
  auto task = std::make_shared<std::packaged_task<ReturnType()>>(
      std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
  auto wrapper = TaskWrapper([task]() { (*task)(); });

  {
    std::lock_guard<std::mutex> lock(m_queue_mutex);
    if (m_stop.load()) {
      m_stats.total_rejected++;
      throw std::runtime_error("ThreadPool has been stopped");
    }
    m_tasks.emplace(std::move(wrapper));
    m_stats.total_submitted++;
  }
  m_condition.notify_one();
  return task->get_future();
}
}  // namespace thread
}  // namespace dry