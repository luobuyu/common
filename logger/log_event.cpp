#include "log_event.h"

#include <unistd.h>

namespace dry {
namespace logger {

const std::string &LevelToString(LogLevel level) {
  static const std::string debug_str = "DEBUG";
  static const std::string info_str = "INFO";
  static const std::string warn_str = "WARN";
  static const std::string error_str = "ERROR";
  static const std::string off_str = "OFF";

  switch (level) {
    case DEBUG:
      return debug_str;
    case INFO:
      return info_str;
    case WARN:
      return warn_str;
    case ERROR:
      return error_str;
    case OFF:
      return off_str;
    default:
      return debug_str;
  }
}

LogEvent::LogEvent(logger::LogLevel log_level, std::string_view module_name,
                   std::string_view file_name, std::string_view function_name, uint32_t line_id,
                   std::string log_msg)
    : timestamp(std::chrono::system_clock::now()),
      log_level(log_level),
      module_name(module_name),
      process_id(getpid()),
      thread_id(std::this_thread::get_id()),
      coroutine_id(0),
      file_name(file_name),
      function_name(function_name),
      line_id(line_id),
      log_msg(std::move(log_msg)){};

std::ostream &operator<<(std::ostream &os, const logger::LogEvent &log_event) {
  os << dry::GetTimeWithMs(log_event.timestamp, "%Y-%m-%d %H:%M:%S") << " ["
     << LevelToString(log_event.log_level) << "] "
     << "[" << std::to_string(log_event.process_id) << ", " << log_event.thread_id << ", "
     << log_event.coroutine_id << "] "
     << "[" << log_event.module_name << "] "
     << "[" << log_event.file_name << ":" << log_event.function_name << ":"
     << log_event.line_id << "] " << log_event.log_msg << "\n";
  return os;
}
};  // namespace logger
}  // namespace dry