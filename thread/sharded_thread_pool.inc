#pragma once

#include <future>

#include "thread/sharded_thread_pool.h"

namespace dry {
namespace thread {

template <typename F, typename... Args>
auto ShardedThreadPool::SubmitOrdered(uint64_t key, F&& func, Args&&... args)
    -> std::future<std::invoke_result_t<F, Args...>> {
  using ReturnType = std::invoke_result_t<F, Args...>;
  if (m_stopped.load(std::memory_order_relaxed)) {
    return std::future<ReturnType>();
  }
  size_t shard_idx = key % m_shards.size();
  return m_shards[shard_idx]->Submit(std::forward<F>(func),
                                     std::forward<Args>(args)...);
}

template <typename F, typename... Args>
auto ShardedThreadPool::Submit(F&& func, Args&&... args)
    -> std::future<std::invoke_result_t<F, Args...>> {
  using ReturnType = std::invoke_result_t<F, Args...>;
  if (m_stopped.load(std::memory_order_relaxed)) {
    return std::future<ReturnType>();
  }
  size_t shard_idx = m_next++ % m_shards.size();
  return m_shards[shard_idx]->Submit(std::forward<F>(func),
                                     std::forward<Args>(args)...);
}
}  // namespace thread

}  // namespace dry
