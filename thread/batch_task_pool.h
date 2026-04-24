#pragma once

#include <future>
#include <vector>

#include "common/thread/thread_pool.h"

namespace dry {
namespace thread {

/**
 * @brief 基于 ThreadPool 的批量任务池
 *
 * 用于在一次请求中并发提交多个任务，统一等待所有任务完成并收集返回值。
 * 任务函数必须返回 int，0 表示成功，非 0 表示失败。
 *
 * 用法示例：
 *   BatchTaskPool batch(pool);
 *   batch.spawn([&] { return fill_player(...); });
 *   batch.spawn([&] { return get_stage_conf(...); });
 *   int ret = batch.wait();  // 等待所有任务完成，返回第一个非 0 错误码
 *
 * NOTE: BatchTaskPool 是一次性对象，用完即销毁，不可复用。
 */
class BatchTaskPool final {
 public:
  explicit BatchTaskPool(ThreadPool& pool) : m_pool(pool) {}

  ~BatchTaskPool() = default;

  BatchTaskPool(const BatchTaskPool&) = delete;
  BatchTaskPool(BatchTaskPool&&) = delete;
  BatchTaskPool& operator=(const BatchTaskPool&) = delete;
  BatchTaskPool& operator=(BatchTaskPool&&) = delete;

  /**
   * 添加一个任务，任务函数必须返回 int
   * 支持链式调用：batch.spawn(...).spawn(...).spawn(...)
   */
  template <typename Func, typename... Args>
  BatchTaskPool& spawn(Func&& func, Args&&... args);

  /**
   * 等待所有任务完成
   * @return 所有任务均成功返回 0，否则返回第一个非 0 的错误码
   */
  int wait();

  /**
   * 返回已提交的任务数量
   */
  std::size_t size() const { return m_futures.size(); }

 private:
  ThreadPool& m_pool;
  std::vector<std::future<int>> m_futures;
};

}  // namespace thread
}  // namespace dry

#include "batch_task_pool.inc"
