#ifndef LOG_BLOCKING_QUEUE_H
#define LOG_BLOCKING_QUEUE_H
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <vector>

namespace dry {
namespace logger {
template <class T>
class CircleQueue {
 public:
  CircleQueue() = default;
  explicit CircleQueue(int max_size);
  bool Full() const;
  bool Empty() const;
  std::size_t Size() const;
  void Push(T &&item);
  void Push(const T &item);
  T &Front();
  void PopFront();
  void Resize(int size);

 private:
  uint32_t m_max_size = 0;
  uint32_t m_head = 0;
  uint32_t m_tail = 0;
  std::vector<T> m_vec;
};

template <class T>
class BlockingQueue {
 public:
  BlockingQueue();
  explicit BlockingQueue(int size);
  ~BlockingQueue();
  void Push(T &&item);
  void Push(const T &item);
  bool TryPop(T &item);
  bool Pop(T &item);
  // 带超时的 pop，超时返回 false 且 item 不变，队列 Stop+空 时也返回 false
  bool PopWithTimeout(T &item, std::chrono::milliseconds timeout);

  /**
   * @brief 带超时的批量弹出，一次加锁，等第一条到来后尽可能多弹出（最多
   * max_count 条）
   * @param[out] out 输出容器，弹出的元素 move-append 到末尾
   * @param max_count 最多弹出的数量
   * @param timeout 队列为空时等待第一条到来的最长时间
   * @return 实际弹出的数量，0 表示超时或队列已 Stop 且为空
   */
  std::size_t BatchPopWithTimeout(std::vector<T> &out, std::size_t max_count,
                                  std::chrono::milliseconds timeout);

  int Size() const;
  void Resize(int size);
  bool Empty();

  void Stop();
  bool IsStopping() const;

 private:
  logger::CircleQueue<T> m_queue;
  mutable std::mutex m_mtx;
  std::condition_variable m_cond_producer;
  std::condition_variable m_cond_comsumer;
  std::atomic_bool m_is_stopping;
};
}  // namespace logger
}  // namespace dry

#include "blocking_queue.inc"
#endif