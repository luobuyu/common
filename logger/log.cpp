#include "log.h"
#include "../dry/dry_time.h"
#include <iostream>
#include <csignal>

namespace logger
{

    /*********************************************************/
    /****************Common*******************************/
    /*********************************************************/

    void coredumpHandler(int signal_no)
    {
        LOG_ERROR("progress received invalid signal, will exit");
        if (Logger::getInstance() != NULL)
        {
            for (auto &sink : Logger::getInstance()->getLogSinks())
            {
                sink->flush();
            }
        }
        signal(signal_no, SIG_DFL);
        raise(signal_no);
    }

    bool Logger::openLog()
    {
        return Logger::getInstance() != nullptr && Logger::is_open_log;
    }

    bool Logger::shouldLog(logger::LogLevel level)
    {
        return level <= Logger::log_level;
    }

    std::string Logger::getModuleName()
    {
        return Logger::m_module_name;
    }

    /*********************************************************/
    /****************Logger*******************************/
    /*********************************************************/
    Logger *Logger::logger_ptr = nullptr;
    logger::LogLevel Logger::log_level = logger::LogLevel::OFF;
    bool Logger::is_open_log = false;
    std::string Logger::m_module_name = "";

    void Logger::registerCoredumpHandler()
    {
        signal(SIGSEGV, coredumpHandler);
        signal(SIGABRT, coredumpHandler);
        signal(SIGTERM, coredumpHandler);
        signal(SIGKILL, coredumpHandler);
        signal(SIGINT, coredumpHandler);
        signal(SIGSTKFLT, coredumpHandler);
        signal(SIGHUP, coredumpHandler);
        // ignore SIGPIPE
        signal(SIGPIPE, SIG_IGN);
    }

    void Logger::init(const logger::LogLevel &log_level, const std::string &module_name, const logger::LogSink::LogSinkPtr &log_sink, const logger::LoggerFormat::LoggerFormatPtr& log_format)
    {
        if (log_level == LogLevel::OFF) return;
        Logger::log_level = log_level;
        Logger::is_open_log = true;
        Logger::m_module_name = module_name;
        m_log_format = log_format;
        m_log_sinks.emplace_back(log_sink);
        registerCoredumpHandler();
    }
    void Logger::init(const logger::LogLevel &log_level, const std::string &module_name, const std::vector<logger::LogSink::LogSinkPtr> &log_sinks, const logger::LoggerFormat::LoggerFormatPtr &log_format)
    {
        if (log_level == LogLevel::OFF)
            return;
        Logger::log_level = log_level;
        Logger::is_open_log = true;
        Logger::m_module_name = module_name;
        m_log_format = log_format;
        m_log_sinks = std::move(log_sinks);
        registerCoredumpHandler();
    }

    std::vector<std::shared_ptr<logger::LogSink>> Logger::getLogSinks()
    {
        return m_log_sinks;
    }

    void Logger::addSink(const logger::LogSink::LogSinkPtr &log_sink)
    {
        m_log_sinks.emplace_back(log_sink);
    }

    Logger *Logger::getInstance()
    {
        return logger_ptr;
    }

    Logger::~Logger()
    {
        for (auto &sink : m_log_sinks)
        {
            sink->flush();
        }
    }

    /*********************************************************/
    /****************SyncLogger*******************************/
    /*********************************************************/
    
    SyncLogger *SyncLogger::getInstance()
    {
        static SyncLogger sync_logger;
        logger_ptr = &sync_logger;
        return &sync_logger;
    }

    void SyncLogger::log(const logger::LogEvent &log_event)
    {
        for (auto &sink : m_log_sinks)
        {
            sink->sink(log_event, m_log_format);
        }
    }
    void logger::SyncLogger::shutDownNow()
    {
    }

    /*********************************************************/
    /****************AsyncLogger*******************************/
    /*********************************************************/

    AsyncLogger *AsyncLogger::getInstance()
    {
        static AsyncLogger async_logger;
        logger_ptr = &async_logger;
        return &async_logger;
    }

    AsyncLogger::~AsyncLogger()
    {
        m_logs.waitStop();
        if (m_async_thread.joinable())
            m_async_thread.join();
        m_logs.stop();
    }

    void logger::AsyncLogger::init(const logger::LogLevel &log_level, const std::string &module_name, const logger::LogSink::LogSinkPtr &log_sink, const logger::LoggerFormat::LoggerFormatPtr &log_format, int queue_size)
    {
        if(log_level == OFF)
            return;
        Logger::log_level = log_level;
        Logger::is_open_log = true;
        Logger::m_module_name = module_name;
        m_log_sinks.emplace_back(log_sink);
        m_logs.resize(queue_size);
        m_log_format = log_format;
        m_async_thread = std::thread(&AsyncLogger::bgLogLoop, AsyncLogger::getInstance());
        registerCoredumpHandler();
    }

    void logger::AsyncLogger::init(const logger::LogLevel &log_level, const std::string &module_name, const std::vector<logger::LogSink::LogSinkPtr> &log_sinks, const logger::LoggerFormat::LoggerFormatPtr &log_format, int queue_size)
    {
        if (log_level == OFF)
            return;
        Logger::log_level = log_level;
        Logger::is_open_log = true;
        Logger::m_module_name = module_name;
        m_log_sinks = std::move(log_sinks);
        m_logs.resize(queue_size);
        m_log_format = log_format;
        m_async_thread = std::thread(&AsyncLogger::bgLogLoop, AsyncLogger::getInstance());
        registerCoredumpHandler();
    }

    // 另起一个线程运行
    void AsyncLogger::bgLogLoop()
    {
        while (true)
        {
            logger::LogEvent log_event;

            if (!m_logs.pop(log_event)) break;
            if (log_event.m_log_msg.empty())
                continue;
            for (auto &sink : m_log_sinks)
            {
                sink->sink(log_event, m_log_format);
            }
        }
    }

    void AsyncLogger::log(const logger::LogEvent &log_event)
    {
        m_logs.push(std::move(log_event));
    }

    void logger::AsyncLogger::shutDownNow()
    {
        m_logs.stop();
        if (m_async_thread.joinable())
            m_async_thread.join();
    }
};