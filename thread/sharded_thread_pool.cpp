#include "thread/sharded_thread_pool.h"

namespace dry {
namespace thread {

ShardedThreadPool::ShardedThreadPool(size_t num_shards) {
  for (size_t i = 0; i < num_shards; ++i) {
    m_shards.emplace_back(std::make_unique<ThreadPool>(1));
  }
}

ShardedThreadPool::~ShardedThreadPool() {
  if (!m_stopped.load(std::memory_order_relaxed)) {
    stop();
  }
}

/// 停止所有 worker（等待队列中的任务执行完毕）
void ShardedThreadPool::stop() {
  if (m_stopped.exchange(true, std::memory_order_acq_rel)) {
    return;
  }
  for (auto& shard : m_shards) {
    shard->stop();
  }
}

/// 获取所有 shard 的总队列长度
size_t ShardedThreadPool::TotalQueueSize() const {
  size_t total = 0;
  for (const auto& shard : m_shards) {
    total += shard->GetQueueSize();
  }
  return total;
}

/// 设置最大排队等待时间（毫秒），传递给所有子 shard
void ShardedThreadPool::SetMaxWaitTimeMs(uint64_t max_wait_time_ms) {
  for (auto& shard : m_shards) {
    shard->SetMaxWaitTimeMs(max_wait_time_ms);
  }
}

ShardedThreadPool::Stats ShardedThreadPool::GetStats() const {
  Stats stats;
  for (const auto& shard : m_shards) {
    stats += shard->GetStats();
  }
  return stats;
}

}  // namespace thread

}  // namespace dry