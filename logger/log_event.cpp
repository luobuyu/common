#include "log_event.h"

#include <unistd.h>

namespace logger {

const std::string &levelToString(LogLevel level) {
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
                   std::string_view file_name, std::string_view function_name,
                   uint32_t line_id, std::string log_msg)
    : m_timestamp(dry::clock::now()),
      m_log_level(log_level),
      m_module_name(module_name),
      m_process_id(getpid()),
      m_thread_id(std::this_thread::get_id()),
      m_coroutine_id(0),
      m_file_name(file_name),
      m_function_name(function_name),
      m_line_id(line_id),
      m_log_msg(std::move(log_msg)){};

std::ostream &operator<<(std::ostream &os, const logger::LogEvent &log_event) {
  os << dry::getTimeWithMs(log_event.m_timestamp, "%Y-%m-%d %H:%M:%S") << " ["
     << levelToString(log_event.m_log_level) << "] "
     << "[" << std::to_string(log_event.m_process_id) << ", "
     << log_event.m_thread_id << ", " << log_event.m_coroutine_id << "] "
     << "[" << log_event.m_module_name << "] "
     << "[" << log_event.m_file_name << ":" << log_event.m_function_name << ":"
     << log_event.m_line_id << "] " << log_event.m_log_msg << "\n";
  return os;
}
};  // namespace logger