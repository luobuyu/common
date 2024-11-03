#ifndef LOG_LOG_H
#define LOG_LOG_H
#include <stdint.h>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <memory>
#include <chrono>
#include "../dry/dry_time.h"
#include "blocking_queue.h"
#include "sink.h"
#include "log_format.h"
#ifdef FMT_FOUND
#include <fmt/core.h>
#include <fmt/printf.h>
#endif

namespace logger
{
    template <typename... Args>
    std::string formatString(const char *str, Args &&...args)
    {
        const int MAX_SIZE = 4096; // 最大长度限制
        const std::string truncation_message = " ... It's longer than " + std::to_string(MAX_SIZE) + ", so it's truncated.";
        bool flag = false;
        int size = snprintf(nullptr, 0, str, args...);
        if (size <= 0)
            return "";
#ifdef FMT_FOUND
        std::string result = fmt::sprintf(str, std::forward<Args>(args)...);
        flag = (result.length() > MAX_SIZE);
#else
        if(size > MAX_SIZE) {
            flag = true;
            size = MAX_SIZE;
        }
        std::string result(size, '\0');
        snprintf(result.data(), result.size() + 1, str, args...);
#endif
        if (flag)
        {
            result.resize(MAX_SIZE);
            result += truncation_message;
        }
        return result;
    }

    void coredumpHandler(int signal_no);

#define LOG_FMT(level, str, ...)                                                                                                                              \
    do                                                                                                                                                        \
    {                                                                                                                                                         \
        if (logger::Logger::openLog() && logger::Logger::shouldLog(level))                                                                                    \
        {                                                                                                                                                     \
            logger::Logger::getInstance()->log(logger::LogEvent(level, logger::Logger::getModuleName(), __FILE__, __func__, __LINE__, logger::formatString(str, ##__VA_ARGS__))); \
        }                                                                                                                                                     \
    } while (0)

// 宏调用 log，传入 level 之类的
// 1
#define LOG_DEBUG(str, ...) \
    LOG_FMT(logger::LogLevel::DEBUG, str, ##__VA_ARGS__)
// 2
#define LOG_INFO(str, ...) \
    LOG_FMT(logger::LogLevel::INFO, str, ##__VA_ARGS__)
// 3
#define LOG_WARN(str, ...) \
    LOG_FMT(logger::LogLevel::WARN, str, ##__VA_ARGS__)
// 4
#define LOG_ERROR(str, ...) \
    LOG_FMT(logger::LogLevel::ERROR, str, ##__VA_ARGS__)
// 5
#define LOG_OFF(str, ...) \
    LOG_FMT(logger::LogLevel::OFF, str, ##__VA_ARGS__)

    // 父类
    class Logger
    {
    public:
        static Logger *getInstance();
        Logger(const Logger &) = delete;
        Logger &operator=(const Logger &) = delete;
        static bool openLog();
        static bool shouldLog(logger::LogLevel level);
        static std::string getModuleName();
        void addSink(const logger::LogSink::LogSinkPtr &log_sink);
        void init(const logger::LogLevel &log_level, const std::string &module_name, const logger::LogSink::LogSinkPtr &log_sink, const logger::LoggerFormat::LoggerFormatPtr& log_format = logger::LoggerFormat::LoggerFormatPtr(new LoggerFormat()));
        void init(const logger::LogLevel &log_level, const std::string &module_name, const std::vector<logger::LogSink::LogSinkPtr> &log_sinks, const logger::LoggerFormat::LoggerFormatPtr &log_format = logger::LoggerFormat::LoggerFormatPtr(new LoggerFormat()));
        std::vector<std::shared_ptr<logger::LogSink>> getLogSinks();
        void registerCoredumpHandler();

        // 需要重写，同步写日志和异步写日志方法
        virtual void shutDownNow() = 0;
        virtual void log(const logger::LogEvent &log_event) = 0;
        ~Logger();

    protected:
        Logger() = default;
        static bool is_open_log; // 是否打开了log
        static logger::LogLevel log_level;
        static Logger *logger_ptr;
        static std::string m_module_name;
        std::vector<std::shared_ptr<logger::LogSink>> m_log_sinks;
        LoggerFormat::LoggerFormatPtr m_log_format;
    };

    class SyncLogger : public Logger
    {
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
    class AsyncLogger : public Logger
    {
    public:
        void bgLogLoop();
        static AsyncLogger *getInstance();
        void init(const logger::LogLevel &log_level, const std::string &module_name, const logger::LogSink::LogSinkPtr &log_sink, const logger::LoggerFormat::LoggerFormatPtr &log_format = logger::LoggerFormat::LoggerFormatPtr(new LoggerFormat()), int queue_size = 8192);
        void init(const logger::LogLevel &log_level, const std::string &module_name, const std::vector<logger::LogSink::LogSinkPtr> &log_sinks, const logger::LoggerFormat::LoggerFormatPtr &log_format = logger::LoggerFormat::LoggerFormatPtr(new LoggerFormat()), int queue_size = 8192);

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
};

#endif
