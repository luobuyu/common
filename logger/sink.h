#ifndef LOGGER_SINK_H
#define LOGGER_SINK_H
#include <string>
#include <fstream>
#include <memory>
#include "../dry/dry_time.h"
#include "log_event.h"
#include "log_format.h"

namespace logger
{
    // 基础日志落盘类。
    class LogSink
    {
    public:
        typedef std::shared_ptr<LogSink> LogSinkPtr;
        LogSink() = default;
        virtual ~LogSink() = default;
        virtual void sink(const logger::LogEvent& log_event, LoggerFormat::LoggerFormatPtr& log_format) = 0;
        virtual void flush() = 0;
    };

    // 按照日期和大小分文件
    class FileSink : public LogSink
    {
    public:
        void sink(const logger::LogEvent &log_event, LoggerFormat::LoggerFormatPtr &log_format) override;
        void openNewFile();
        void flush() override;
        FileSink(const std::string &file_path, uint32_t max_size = 10 * 1024 * 1024, dry::clock::duration rotate_inteval = std::chrono::hours(1));

    private:
        std::string m_file_path; // 路径，
        uint32_t m_no;
        uint32_t m_max_size;

        std::ofstream m_ofs; // 当前文件
        dry::clock::duration m_rotate_inteval;
        dry::clock::time_point m_next_start;
    };

    class StdoutSink: public LogSink
    {
    public:
        void sink(const logger::LogEvent &log_event, LoggerFormat::LoggerFormatPtr &log_format) override;
        void flush() override;

    private:
    };
} // namespace logger

#endif