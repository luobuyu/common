#include "blocking_queue.h"

namespace logger {
/*********************************************************/
/****************CircleQueue*******************************/
/*********************************************************/
template <class T>
CircleQueue<T>::CircleQueue(int max_size)
    : m_max_size(max_size + 1), m_vec(m_max_size) {}

template <class T>
bool CircleQueue<T>::full() const {
  return (m_tail + 1) % m_max_size == m_head;
}

template <class T>
bool CircleQueue<T>::empty() const {
  return m_head == m_tail;
}

template <class T>
std::size_t CircleQueue<T>::size() const {
  return (m_tail - m_head + m_max_size) % m_max_size;
}

template <class T>
void CircleQueue<T>::push(T &&item) {
  m_vec[m_tail] = std::move(item);
  m_tail = (m_tail + 1) % m_max_size;
}

template <class T>
void CircleQueue<T>::push(const T &item) {
  m_vec[m_tail] = item;
  m_tail = (m_tail + 1) % m_max_size;
}

template <class T>
T &CircleQueue<T>::front() {
  return m_vec[m_head];
}

template <class T>
void CircleQueue<T>::popFront() {
  m_head = (m_head + 1) % m_max_size;
}

template <class T>
void logger::CircleQueue<T>::resize(int size) {
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
void BlockingQueue<T>::push(T &&item) {
  {
    std::unique_lock<std::mutex> lock(m_mtx);
    m_cond_producer.wait(lock,
                         [&]() { return !m_queue.full() || m_is_stopping; });
    if (m_is_stopping) return;  // 停止后不再接受新数据
    m_queue.push(std::move(item));
  }
  m_cond_comsumer.notify_one();
}
template <class T>
void BlockingQueue<T>::push(const T &item) {
  {
    std::unique_lock<std::mutex> lock(m_mtx);
    m_cond_producer.wait(lock,
                         [&]() { return !m_queue.full() || m_is_stopping; });
    if (m_is_stopping) return;  // 停止后不再接受新数据
    m_queue.push(std::move(item));
  }
  m_cond_comsumer.notify_one();
}
template <class T>
bool BlockingQueue<T>::tryPop(T &item) {
  std::lock_guard<std::mutex> lock(m_mtx);
  if (m_queue.empty()) return false;
  item = std::move(m_queue.front());
  m_queue.popFront();
  m_cond_producer.notify_one();
  return true;
}
template <class T>
bool BlockingQueue<T>::pop(T &item) {
  std::unique_lock<std::mutex> lock(m_mtx);
  m_cond_comsumer.wait(lock,
                       [&]() { return !m_queue.empty() || m_is_stopping; });
  if (m_is_stopping && m_queue.empty()) return false;
  item = std::move(m_queue.front());
  m_queue.popFront();
  m_cond_producer.notify_one();
  return true;
}
template <class T>
bool BlockingQueue<T>::popWithTimeout(T &item,
                                      std::chrono::milliseconds timeout) {
  std::unique_lock<std::mutex> lock(m_mtx);
  bool got = m_cond_comsumer.wait_for(
      lock, timeout, [&]() { return !m_queue.empty() || m_is_stopping; });
  // got == false: 超时且谓词仍不满足 → 返回 false
  // m_is_stopping && m_queue.empty(): stop 且队列空 → 返回 false
  if (!got || (m_is_stopping && m_queue.empty())) return false;
  item = std::move(m_queue.front());
  m_queue.popFront();
  m_cond_producer.notify_one();
  return true;
}

template <class T>
int BlockingQueue<T>::size() const {
  std::lock_guard<std::mutex> lock(m_mtx);
  return m_queue.size();
}
template <class T>
void BlockingQueue<T>::resize(int size) {
  std::lock_guard<std::mutex> lock(m_mtx);
  m_queue.resize(size);
  m_is_stopping.store(false);
}
template <class T>
bool BlockingQueue<T>::empty() {
  std::lock_guard<std::mutex> lock(m_mtx);
  return m_queue.empty();
}
template <class T>
void BlockingQueue<T>::stop() {
  m_is_stopping.store(true);
  m_cond_comsumer.notify_all();
  m_cond_producer.notify_all();  // 唤醒可能阻塞在 push 的生产者
}
template <class T>
bool BlockingQueue<T>::isStopping() const {
  return m_is_stopping.load();
}
}  // namespace logger