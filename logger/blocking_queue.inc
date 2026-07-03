#include "blocking_queue.h"

namespace dry {
namespace logger {
/*********************************************************/
/****************CircleQueue*******************************/
/*********************************************************/
template <class T>
CircleQueue<T>::CircleQueue(int max_size)
    : m_max_size(max_size + 1), m_vec(m_max_size) {}

template <class T>
bool CircleQueue<T>::Full() const {
  return (m_tail + 1) % m_max_size == m_head;
}

template <class T>
bool CircleQueue<T>::Empty() const {
  return m_head == m_tail;
}

template <class T>
std::size_t CircleQueue<T>::Size() const {
  return (m_tail - m_head + m_max_size) % m_max_size;
}

template <class T>
void CircleQueue<T>::Push(T &&item) {
  m_vec[m_tail] = std::move(item);
  m_tail = (m_tail + 1) % m_max_size;
}

template <class T>
void CircleQueue<T>::Push(const T &item) {
  m_vec[m_tail] = item;
  m_tail = (m_tail + 1) % m_max_size;
}

template <class T>
T &CircleQueue<T>::Front() {
  return m_vec[m_head];
}

template <class T>
void CircleQueue<T>::PopFront() {
  m_head = (m_head + 1) % m_max_size;
}

template <class T>
void logger::CircleQueue<T>::Resize(int size) {
  m_max_size = size + 1;
  m_vec.resize(m_max_size);
  m_head = m_tail = 0;
}

/*********************************************************/
/****************BlockingQueue*******************************/
/*********************************************************/
template <class T>
BlockingQueue<T>::BlockingQueue() : m_queue(1024), m_is_stopping(false) {}
template <class T>
BlockingQueue<T>::BlockingQueue(int max_size)
    : m_queue(max_size), m_is_stopping(false) {}
template <class T>
BlockingQueue<T>::~BlockingQueue() {
  m_cond_comsumer.notify_all();
}
template <class T>
void BlockingQueue<T>::Push(T &&item) {
  {
    std::unique_lock<std::mutex> lock(m_mtx);
    m_cond_producer.wait(lock,
                         [&]() { return !m_queue.Full() || m_is_stopping; });
    if (m_is_stopping) return;  // 停止后不再接受新数据
    m_queue.push(std::move(item));
  }
  m_cond_comsumer.notify_one();
}
template <class T>
void BlockingQueue<T>::Push(const T &item) {
  {
    std::unique_lock<std::mutex> lock(m_mtx);
    m_cond_producer.wait(lock,
                         [&]() { return !m_queue.Full() || m_is_stopping; });
    if (m_is_stopping) return;  // 停止后不再接受新数据
    m_queue.push(item);
  }
  m_cond_comsumer.notify_one();
}
template <class T>
bool BlockingQueue<T>::TryPop(T &item) {
  std::lock_guard<std::mutex> lock(m_mtx);
  if (m_queue.empty()) return false;
  item = std::move(m_queue.front());
  m_queue.PopFront();
  m_cond_producer.notify_one();
  return true;
}
template <class T>
bool BlockingQueue<T>::Pop(T &item) {
  std::unique_lock<std::mutex> lock(m_mtx);
  m_cond_comsumer.wait(lock,
                       [&]() { return !m_queue.empty() || m_is_stopping; });
  if (m_is_stopping && m_queue.empty()) return false;
  item = std::move(m_queue.front());
  m_queue.PopFront();
  m_cond_producer.notify_one();
  return true;
}
template <class T>
bool BlockingQueue<T>::PopWithTimeout(T &item,
                                      std::chrono::milliseconds timeout) {
  std::unique_lock<std::mutex> lock(m_mtx);
  bool got = m_cond_comsumer.wait_for(
      lock, timeout, [&]() { return !m_queue.empty() || m_is_stopping; });
  // got == false: 超时且谓词仍不满足 → 返回 false
  // m_is_stopping && m_queue.empty(): Stop 且队列空 → 返回 false
  if (!got || (m_is_stopping && m_queue.empty())) return false;
  item = std::move(m_queue.front());
  m_queue.PopFront();
  m_cond_producer.notify_one();
  return true;
}

template <class T>
int BlockingQueue<T>::Size() const {
  std::lock_guard<std::mutex> lock(m_mtx);
  return m_queue.size();
}
template <class T>
void BlockingQueue<T>::Resize(int size) {
  std::lock_guard<std::mutex> lock(m_mtx);
  m_queue.resize(size);
  m_is_stopping.store(false);
}
template <class T>
bool BlockingQueue<T>::Empty() {
  std::lock_guard<std::mutex> lock(m_mtx);
  return m_queue.empty();
}
template <class T>
void BlockingQueue<T>::Stop() {
  m_is_stopping.store(true);
  m_cond_comsumer.notify_all();
  m_cond_producer.notify_all();  // 唤醒可能阻塞在 push 的生产者
}
template <class T>
bool BlockingQueue<T>::IsStopping() const {
  return m_is_stopping.load();
}
template <class T>
std::size_t BlockingQueue<T>::BatchPopWithTimeout(
    std::vector<T> &out, std::size_t max_count,
    std::chrono::milliseconds timeout) {
  std::unique_lock<std::mutex> lock(m_mtx);
  // 等第一条到来（或超时/Stop）
  bool got = m_cond_comsumer.wait_for(
      lock, timeout, [&]() { return !m_queue.empty() || m_is_stopping; });
  if (!got || (m_is_stopping && m_queue.empty())) return 0;

  // 一次加锁，尽可能多弹出
  std::size_t count = 0;
  while (count < max_count && !m_queue.empty()) {
    out.emplace_back(std::move(m_queue.front()));
    m_queue.PopFront();
    ++count;
  }
  if (count > 0) {
    m_cond_producer.notify_all();
  }
  return count;
}
}  // namespace logger
}  // namespace dry