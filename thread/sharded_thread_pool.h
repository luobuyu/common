#pragma once
#include <atomic>
#include <cstddef>
#include <future>
#include <memory>
#include <queue>
#include <thread>
#include <vector>

#include "thread/thread_pool.h"

namespace dry {
namespace thread {
class ShardedThreadPool {
 public:
  // 线程池统计信息
  using Stats = ThreadPool::Stats;

  explicit ShardedThreadPool(size_t num_shards);
  ~ShardedThreadPool();

  // 禁止拷贝和移动
  ShardedThreadPool(const ShardedThreadPool&) = delete;
  ShardedThreadPool& operator=(const ShardedThreadPool&) = delete;

  /// @brief 保序投递：同 key 进同 shard，FIFO 串行执行
  /// @param key 分片键（通常是 connection_id）
  /// @param task 任务
  /// @note HttpServer 使用此接口，保证同连接的请求按顺序处理
  template <typename F, typename... Args>
  auto submitOrdered(uint64_t key, F&& func, Args&&... args)
      -> std::future<std::invoke_result_t<F, Args...>>;

  /// @brief 轮询投递：不保序，最大并发
  /// @param task 任务
  /// @note RpcServer 使用此接口，不同请求可并发处理
  template <typename F, typename... Args>
  auto submit(F&& func, Args&&... args)
      -> std::future<std::invoke_result_t<F, Args...>>;

  /// 停止所有 worker（等待队列中的任务执行完毕）
  void stop();

  /// 获取 shard 数量
  size_t shardCount() const { return m_shards.size(); }

  /// 获取指定 shard 的队列长度
  size_t queueSize(size_t shard_idx) const {
    return m_shards.at(shard_idx)->getQueueSize();
  }

  /// 获取所有 shard 的总队列长度
  size_t totalQueueSize() const;

  /// 设置最大排队等待时间（毫秒），超过则丢弃任务，0 表示不限制
  void setMaxWaitTimeMs(uint64_t max_wait_time_ms);

  /// 获取所有 shard 的汇总统计信息
  Stats getStats() const;

 private:
  std::vector<std::unique_ptr<ThreadPool>> m_shards;  // 分片线程池
  std::atomic<size_t> m_next{0};                      // 轮询计数器
  std::atomic<bool> m_stopped{false};                 // 防止重复 stop
};

}  // namespace thread
}  // namespace dry

#include "thread/sharded_thread_pool.inc"