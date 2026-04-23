#ifndef LOGGER_LOG_EVENT
#define LOGGER_LOG_EVENT
#include <iostream>
#include <string_view>
#include <thread>

#include "../dry_core/dry_time.h"
namespace dry {
namespace logger {
enum LogLevel {
  DEBUG = 1,
  INFO = 2,
  WARN = 3,
  ERROR = 4,
  OFF = 5  // don't print log
};
const std::string &levelToString(LogLevel level);

// 单条日志的具体需要包含哪些内容
// 2024-10-04 12:34:56.789 [INFO] [ThreadID: 12345] [AuthModule] main.cpp:42
// [时间戳] [日志级别] [模块名] [线程ID] [文件名:行号] [callgraphid]
struct LogEvent {
 public:
  LogEvent(logger::LogLevel log_level, std::string_view module_name,
           std::string_view file_name, std::string_view function_name,
           uint32_t line_id, std::string log_msg);
  friend std::ostream &operator<<(std::ostream &os,
                                  const logger::LogEvent &log_event);
  LogEvent() = default;

  dry::clock::time_point m_timestamp;
  logger::LogLevel m_log_level;
  std::string_view m_module_name;
  uint32_t m_process_id;
  std::thread::id m_thread_id;
  uint32_t m_coroutine_id;
  std::string_view m_file_name;
  std::string_view m_function_name;
  uint32_t m_line_id;
  std::string m_log_msg;
};
};  // namespace logger
}  // namespace dry

#endif