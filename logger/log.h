#ifndef LOG_LOG_H
#define LOG_LOG_H
#include <stdint.h>

#include <memory>
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
  const int MAX_SIZE = 4096;  // 最大长度限制
  const std::string truncation_message = " ... It's longer than " +
                                         std::to_string(MAX_SIZE) +
                                         ", so it's truncated.";
  bool flag = false;
  int size = snprintf(nullptr, 0, str, args...);
  if (size <= 0) return "";
#ifdef FMT_FOUND
  std::string result = fmt::sprintf(str, std::forward<Args>(args)...);
  flag = (result.length() > MAX_SIZE);
#else
  if (size > MAX_SIZE) {
    flag = true;
    size = MAX_SIZE;
  }
  std::string result(size, '\0');
  snprintf(result.data(), result.size() + 1, str, args...);
#endif
  if (flag) {
    result.resize(MAX_SIZE);
    result += truncation_message;
  }
  return result;
}

/**
 * @brief 核心转储信号处理函数
 */
void coredumpHandler(int signal_no);

// =============================================================================
// 日志宏定义
// =============================================================================

#define LOG_FMT(level, str, ...)                                         \
  do {                                                                   \
    if (logger::Logger::openLog() && logger::Logger::shouldLog(level)) { \
      logger::Logger::getInstance()->log(logger::LogEvent(               \
          level, logger::Logger::getModuleName(), __FILE__, __func__,    \
          __LINE__, logger::formatString(str, ##__VA_ARGS__)));          \
    }                                                                    \
  } while (0)

// 日志级别宏定义
#define LOG_DEBUG(str, ...) LOG_FMT(logger::LogLevel::DEBUG, str, ##__VA_ARGS__)
#define LOG_INFO(str, ...) LOG_FMT(logger::LogLevel::INFO, str, ##__VA_ARGS__)
#define LOG_WARN(str, ...) LOG_FMT(logger::LogLevel::WARN, str, ##__VA_ARGS__)
#define LOG_ERROR(str, ...) LOG_FMT(logger::LogLevel::ERROR, str, ##__VA_ARGS__)
#define LOG_OFF(str, ...) LOG_FMT(logger::LogLevel::OFF, str, ##__VA_ARGS__)

// =============================================================================
// Logger类定义
// =============================================================================
class Logger {
 public:
  static Logger *getInstance();
  Logger(const Logger &) = delete;
  Logger &operator=(const Logger &) = delete;
  static bool openLog();
  static bool shouldLog(logger::LogLevel level);
  static std::string getModuleName();
  void addSink(const logger::LogSink::LogSinkPtr &log_sink);
  void init(const logger::LogLevel &log_level, const std::string &module_name,
            const logger::LogSink::LogSinkPtr &log_sink,
            const logger::LoggerFormat::LoggerFormatPtr &log_format =
                logger::LoggerFormat::LoggerFormatPtr(new LoggerFormat()));
  void init(const logger::LogLevel &log_level, const std::string &module_name,
            const std::vector<logger::LogSink::LogSinkPtr> &log_sinks,
            const logger::LoggerFormat::LoggerFormatPtr &log_format =
                logger::LoggerFormat::LoggerFormatPtr(new LoggerFormat()));
  std::vector<std::shared_ptr<logger::LogSink>> getLogSinks();
  void registerCoredumpHandler();

  // 需要重写，同步写日志和异步写日志方法
  virtual void shutDownNow() = 0;
  virtual void log(const logger::LogEvent &log_event) = 0;
  ~Logger();

 protected:
  Logger() = default;
  static bool is_open_log;  // 是否打开了log
  static logger::LogLevel log_level;
  static Logger *logger_ptr;
  static std::string m_module_name;
  std::vector<std::shared_ptr<logger::LogSink>> m_log_sinks;
  LoggerFormat::LoggerFormatPtr m_log_format;
};

class SyncLogger : public Logger {
 public:
  static SyncLogger *getInstance();
  void log(const logger::LogEvent &log_event) override;
  void shutDownNow() override;
  ~SyncLogger() = default;

 private:
  SyncLogger() = default;
};

// 异步刷盘类，将queue中的日志刷盘
// 如果需要输出到多个地方，可以搞多个刷盘的buffer。
class AsyncLogger : public Logger {
 public:
  void bgLogLoop();
  static AsyncLogger *getInstance();
  void init(const logger::LogLevel &log_level, const std::string &module_name,
            const logger::LogSink::LogSinkPtr &log_sink,
            const logger::LoggerFormat::LoggerFormatPtr &log_format =
                logger::LoggerFormat::LoggerFormatPtr(new LoggerFormat()),
            int queue_size = 8192);
  void init(const logger::LogLevel &log_level, const std::string &module_name,
            const std::vector<logger::LogSink::LogSinkPtr> &log_sinks,
            const logger::LoggerFormat::LoggerFormatPtr &log_format =
                logger::LoggerFormat::LoggerFormatPtr(new LoggerFormat()),
            int queue_size = 8192);

  AsyncLogger(const AsyncLogger &) = delete;
  AsyncLogger &operator=(const AsyncLogger &) = delete;
  void log(const logger::LogEvent &log_event) override;
  void shutDownNow() override;
  ~AsyncLogger();

 private:
  AsyncLogger() = default;

  // 单线程刷盘
  std::thread m_async_thread;
  logger::BlockingQueue<logger::LogEvent> m_logs;
};
};  // namespace logger

#endif
