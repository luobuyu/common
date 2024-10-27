#include "log_event.h"
#include <unistd.h>

namespace logger {

    std::string levelToString(LogLevel level)
    {
        std::string re = "DEBUG";
        switch (level)
        {
        case DEBUG:
            re = "DEBUG";
            return re;

        case INFO:
            re = "INFO";
            return re;

        case WARN:
            re = "WARN";
            return re;

        case ERROR:
            re = "ERROR";
            return re;
        case OFF:
            re = "OFF";
        default:
            return re;
        }
    }

    LogEvent::LogEvent(
        logger::LogLevel log_level,
        std::string module_name,
        std::string file_name,
        std::string funtion_name,
        uint32_t line_id,
        std::string log_msg) : m_timestamp(dry::clock::now()),
                               m_log_level(log_level),
                               m_module_name(module_name),
                               m_process_id(getpid()),
                               m_thread_id(std::this_thread::get_id()),
                               m_coroutine_id(0),
                               m_file_name(file_name),
                               m_funtion_name(funtion_name),
                               m_line_id(line_id),
                               m_log_msg(log_msg) {};

    std::ostream &operator<<(std::ostream &os, const logger::LogEvent &log_event) {
        os << dry::getTime(log_event.m_timestamp, "%Y-%m-%d %H:%M:%S ")
           << "[" << levelToString(log_event.m_log_level) << "] "
           << "[" << std::to_string(log_event.m_process_id) << ", " << log_event.m_thread_id << ", " << log_event.m_coroutine_id << "] "
           << "[" << log_event.m_module_name << "] "
           << "[" << log_event.m_file_name << ":" << log_event.m_funtion_name << ":" << log_event.m_line_id << "] "
           << log_event.m_log_msg << "\n";
        return os;
    }
};