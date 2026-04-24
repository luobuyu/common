#pragma once
#include "common/thread/batch_task_pool.h"

namespace dry {
namespace thread {

template <typename Func, typename... Args>
BatchTaskPool& BatchTaskPool::spawn(Func&& func, Args&&... args) {
  // submit 要求返回类型为 int
  static_assert(std::is_same<int, std::invoke_result_t<Func, Args...>>::value,
                "BatchTaskPool::spawn: task function must return int");
  m_futures.push_back(
      m_pool.submit(std::forward<Func>(func), std::forward<Args>(args)...));
  return *this;
}

inline int BatchTaskPool::wait() {
  int first_err = 0;
  for (auto& f : m_futures) {
    int ret = f.get();
    if (ret != 0 && first_err == 0) {
      first_err = ret;
    }
  }
  return first_err;
}

}  // namespace thread
}  // namespace dry
