#ifndef LOG_LOG_H
#define LOG_LOG_H

#include <chrono>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "blocking_queue.h"
#include "log_format.h"
#include "sink.h"
#ifdef FMT_FOUND
#include <fmt/format.h>
#endif

namespace dry {
namespace logger {

// =============================================================================
// 工具函数
// =============================================================================

/**
 * @brief 格式化字符串工具函数（无参版本）
 * @details 无格式化参数时直接拷贝字符串，避免调用 snprintf
 *          触发 clang/gcc 的 -Wformat-security 告警。
 * @param str 字符串内容（可能含 % 但无对应参数，按字面量原样输出）
 */
inline std::string FormatString(const char *str) {
  return std::string(str ? str : "");
}

/**
 * @brief 格式化字符串工具函数（{} 风格，fmt::format）
 * @details 供 LOG_* 宏使用。需要 fmt 库（FMT_FOUND）。
 *          支持全部 fmt 格式说明：{:.2f} {:>10} {:<5} {:x} 等。
 * @code
 *   FormatString("id={}, name={}", 42, "tom");
 *   FormatString("price={:.2f}", 3.14159);   // → "price=3.14"
 * @endcode
 */
template <typename... Args>
std::string FormatString(const char *str, Args &&...args) {
#ifdef FMT_FOUND
  return fmt::format(fmt::runtime(str), std::forward<Args>(args)...);
#else
  static_assert(sizeof...(Args) < 0,
                "LOG_* requires the fmt library. "
                "Install fmt or upgrade to C++20 for std::format.");
  return {};
#endif
}

// =============================================================================
// Logger 纯抽象接口
// =============================================================================

/**
 * @brief Logger 抽象基类
 * @details 只定义日志写入接口和 Sink/format 管理，不持有任何全局状态。
 *          全局状态（log_level、module_name 等）由 LogManager 管理。
 */
class Logger {
 public:
  Logger(const Logger &) = delete;
  Logger &operator=(const Logger &) = delete;

  /// 添加日志输出目标（线程安全）
  void AddSink(const LogSink::LogSinkPtr &log_sink);

  /// 获取所有 Sink
  const std::vector<LogSink::LogSinkPtr> &GetLogSinks() const;

  /// 写入一条日志（子类实现同步/异步逻辑）
  virtual void Log(LogEvent log_event) = 0;

  /// 退出前的清理操作，子类可重写（如 AsyncLogger 需排空队列）
  virtual void BeforeExit() {}

  virtual ~Logger();

 protected:
  /// 默认构造
  Logger() = default;

  /// 带参构造：直接传入 sinks 和 format，满足 RAII 原则
  Logger(std::vector<LogSink::LogSinkPtr> sinks,
         LoggerFormat::LoggerFormatPtr fmt);

  std::mutex m_sink_mtx;  ///< 保护 m_log_sinks 的并发访问
  std::vector<LogSink::LogSinkPtr> m_log_sinks;
  LoggerFormat::LoggerFormatPtr m_log_format;
};

// =============================================================================
// 同步日志器
// =============================================================================

class SyncLogger : public Logger {
 public:
  SyncLogger() = default;

  /// 带参构造：构造即可用
  SyncLogger(std::vector<LogSink::LogSinkPtr> sinks,
             LoggerFormat::LoggerFormatPtr fmt);

  void Log(LogEvent log_event) override;
  ~SyncLogger() = default;

 private:
  std::mutex m_mtx;  ///< 保护多线程同步写入
};

// =============================================================================
// 异步日志器
// =============================================================================

/**
 * @brief 异步日志器，通过阻塞队列 + 后台线程异步写入
 */
class AsyncLogger : public Logger {
 public:
  AsyncLogger() = default;
  AsyncLogger(const AsyncLogger &) = delete;
  AsyncLogger &operator=(const AsyncLogger &) = delete;

  /// 带参构造：构造即启动后台线程，满足 RAII
  AsyncLogger(std::vector<LogSink::LogSinkPtr> sinks,
              LoggerFormat::LoggerFormatPtr fmt, int queue_size = 8192,
              std::chrono::milliseconds flush_interval =
                  std::chrono::milliseconds(3000));

  /**
   * @brief 启动后台消费线程
   * @param queue_size 阻塞队列大小
   * @param flush_interval 定时刷盘间隔
   */
  void StartBgThread(int queue_size = 8192,
                     std::chrono::milliseconds flush_interval =
                         std::chrono::milliseconds(3000));

  void Log(LogEvent log_event) override;

  /// 退出前排空异步队列并等待消费线程结束
  void BeforeExit() override;

  ~AsyncLogger();

 private:
  /// 后台日志消费循环
  void BgLogLoop();

  std::thread m_async_thread;
  BlockingQueue<LogEvent> m_logs;
  /// 定时刷盘间隔，默认 3 秒
  std::chrono::milliseconds m_flush_interval{3000};
};

}  // namespace logger
}  // namespace dry

#endif
