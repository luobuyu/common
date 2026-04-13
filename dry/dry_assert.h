
#ifndef DRY_ASSERT_H
#define DRY_ASSERT_H

/**
 * @file dry_assert.h
 * @brief 带日志输出的断言宏
 * @details
 *   - Debug 模式：断言失败时先通过 LOG_ERROR 打印日志（能落盘），再 abort
 *   - Release 模式（NDEBUG）：完全消除，零开销
 *
 * @code
 *   DRY_ASSERT(isLoopInThisThread(),
 *              "must be called in EventLoop thread, current=%d, expected=%d",
 *              dry::getThreadId(), m_thread_id);
 * @endcode
 */

#include "dry_log.h"

#ifdef NDEBUG

#define DRY_ASSERT(expr, fmt, ...) ((void)0)

#else

#include <cassert>

/// @brief 带日志的断言宏，失败时先打 LOG_ERROR 再 abort
/// @param expr 断言表达式
/// @param fmt  printf 风格的格式化字符串（描述断言失败的上下文）
/// @param ...  格式化参数
#define DRY_ASSERT(expr, fmt, ...)                                 \
  do {                                                             \
    if (!(expr)) {                                                 \
      LOG_ERROR("ASSERT FAILED: %s | " fmt, #expr, ##__VA_ARGS__); \
      assert(false && #expr);                                      \
    }                                                              \
  } while (0)

#endif  // NDEBUG

#endif  // DRY_ASSERT_H
