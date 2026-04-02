
#ifndef DRY_THREAD_H
#define DRY_THREAD_H

#include <sys/syscall.h>
#include <unistd.h>

namespace dry {

// 获取当前线程的 tid（内核线程 ID），使用 thread_local 缓存避免重复系统调用
inline int getThreadId() {
  static thread_local int tid = static_cast<int>(syscall(SYS_gettid));
  return tid;
}

}  // namespace dry

#endif  // DRY_THREAD_H
