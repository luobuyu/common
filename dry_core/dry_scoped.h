#pragma once

#include <cstdio>     // fprintf
#include <exception>  // std::exception, std::terminate
#include <utility>    // std::forward

namespace dry {

/// RAII 作用域守卫，析构时自动执行回调。
///
/// 用法：
///   auto guard = dry::MakeScopedGuard([&] { /* cleanup */ });
///   guard.SetRelease(true);  // 可选：取消回调
template <typename Fn>
class ScopedGuard final {
 public:
  template <typename L,
            typename std::enable_if<
                !std::is_same<std::remove_reference<std::remove_cv<L>>,
                              ScopedGuard>::value,
                int>::type = 0>
  explicit ScopedGuard(L&& fn) : m_fn(std::forward<L>(fn)), m_released(false) {}

  ~ScopedGuard() {
    if (!m_released) {
      try {
        m_fn();
      } catch (const std::exception& e) {
        // 用 fprintf 而非 LOG_ERROR：底层工具不依赖日志系统
        std::fprintf(stderr, "ScopedGuard caught exception: %s\n", e.what());
        std::terminate();
      } catch (...) {
        std::fprintf(stderr, "ScopedGuard caught unknown exception\n");
        std::terminate();
      }
    }
  }

  ScopedGuard(const ScopedGuard&) = delete;
  ScopedGuard& operator=(const ScopedGuard&) = delete;
  ScopedGuard(ScopedGuard&&) = delete;
  ScopedGuard& operator=(ScopedGuard&&) = delete;

  /// 释放守卫，析构时不再执行回调
  void SetRelease(bool r = true) { m_released = r; }

 private:
  Fn m_fn;
  bool m_released;
};

/// 工厂函数，自动推导模板参数
template <typename Fn>
ScopedGuard<Fn> MakeScopedGuard(Fn&& fn) {
  return ScopedGuard<Fn>(std::forward<Fn>(fn));
}

}  // namespace dry
