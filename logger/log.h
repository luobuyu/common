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
#include <fmt/core.h>
#include <fmt/printf.h>
#endif

namespace logger {

// =============================================================================
// 工具函数
// =============================================================================

/**
 * @brief 格式化字符串工具函数
 * @tparam Args 可变参数类型
 * @param str 格式化字符串
 * @param args 参数列表
 * @return 格式化后的字符串（自动截断超长内容）
 */
template <typename... Args>
std::string formatString(const char *str, Args &&...args) {
  static constexpr int MAX_SIZE = 4096;  // 最大长度限制
  static const std::string truncation_message = " ... It's longer than " +
                                                std::to_string(MAX_SIZE) +
                                                ", so it's truncated.";

#ifdef FMT_FOUND
  std::string result = fmt::sprintf(str, std::forward<Args>(args)...);
  if (result.length() > MAX_SIZE) {
    result.resize(MAX_SIZE);
    result += truncation_message;
  }
  return result;
#else
  // 栈上 buffer 优先，绝大多数日志一次 snprintf 搞定
  char stack_buf[MAX_SIZE + 1];
  int size = snprintf(stack_buf, sizeof(stack_buf), str, args...);
  if (size < 0) return "";

  if (size < static_cast<int>(sizeof(stack_buf))) {
    // 未截断，直接从栈 buffer 构造 string
    return std::string(stack_buf, size);
  }

  // 超长：stack_buf 中已有 MAX_SIZE 字节的有效内容，拼上截断提示
  std::string result(stack_buf, MAX_SIZE);
  result += truncation_message;
  return result;
#endif
}

// =============================================================================
// Logger 纯抽象接口
// =============================================================================

/**
 * @brief Logger 抽象基类
 * @details 只定义日志写入接口和 sink/format 管理，不持有任何全局状态。
 *          全局状态（log_level、module_name 等）由 LogManager 管理。
 */
class Logger {
 public:
  Logger(const Logger &) = delete;
  Logger &operator=(const Logger &) = delete;

  /// 添加日志输出目标（线程安全）
  void addSink(const LogSink::LogSinkPtr &log_sink);

  /// 获取所有 sink
  const std::vector<LogSink::LogSinkPtr> &getLogSinks() const;

  /// 写入一条日志（子类实现同步/异步逻辑）
  virtual void log(LogEvent log_event) = 0;

  /// 退出前的清理操作，子类可重写（如 AsyncLogger 需排空队列）
  virtual void beforeExit() {}

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

  void log(LogEvent log_event) override;
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
  void startBgThread(int queue_size = 8192,
                     std::chrono::milliseconds flush_interval =
                         std::chrono::milliseconds(3000));

  void log(LogEvent log_event) override;

  /// 退出前排空异步队列并等待消费线程结束
  void beforeExit() override;

  ~AsyncLogger();

 private:
  /// 后台日志消费循环
  void bgLogLoop();

  std::thread m_async_thread;
  BlockingQueue<LogEvent> m_logs;
  /// 定时刷盘间隔，默认 3 秒
  std::chrono::milliseconds m_flush_interval{3000};
};

}  // namespace logger

#endif
