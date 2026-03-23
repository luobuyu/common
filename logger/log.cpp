#include "log.h"

#include <csignal>

namespace logger {

/*********************************************************/
/****************Logger Static Members*******************/
/*********************************************************/

void Logger::coredumpHandler(int signal_no) {
  std::cerr << "process received invalid signal, will exit" << std::endl;
  // 这里仅做 flush 尽量保证已有日志写入磁盘
  if (Logger::getInstance() != nullptr) {
    for (auto &sink : Logger::getInstance()->getLogSinks()) {
      sink->flush();
    }
  }
  signal(signal_no, SIG_DFL);
  raise(signal_no);
}

bool Logger::openLog() {
  return Logger::getInstance() != nullptr && Logger::is_open_log;
}

bool Logger::shouldLog(logger::LogLevel level) {
  return level <= Logger::log_level;
}

std::string Logger::getModuleName() { return Logger::m_module_name; }

/*********************************************************/
/****************Logger*******************************/
/*********************************************************/
Logger *Logger::logger_ptr = nullptr;
logger::LogLevel Logger::log_level = logger::LogLevel::OFF;
bool Logger::is_open_log = false;
std::string Logger::m_module_name = "";

void Logger::registerCoredumpHandler() {
#if defined(__linux__) || defined(__APPLE__)
  signal(SIGSEGV, coredumpHandler);
  signal(SIGABRT, coredumpHandler);
  signal(SIGTERM, coredumpHandler);
  // SIGKILL 不可捕获，不注册
  signal(SIGINT, coredumpHandler);
  signal(SIGHUP, coredumpHandler);
  // ignore SIGPIPE
  signal(SIGPIPE, SIG_IGN);
#endif
#if defined(__linux__)
  signal(SIGSTKFLT, coredumpHandler);
#endif
}

void Logger::init(const logger::LogLevel &log_level,
                  const std::string &module_name,
                  const logger::LogSink::LogSinkPtr &log_sink,
                  const logger::LoggerFormat::LoggerFormatPtr &log_format) {
  if (log_level == LogLevel::OFF) return;
  Logger::log_level = log_level;
  Logger::is_open_log = true;
  Logger::m_module_name = module_name;
  m_log_format = log_format;
  m_log_sinks.emplace_back(log_sink);
  registerCoredumpHandler();
}
void Logger::init(const logger::LogLevel &log_level,
                  const std::string &module_name,
                  const std::vector<logger::LogSink::LogSinkPtr> &log_sinks,
                  const logger::LoggerFormat::LoggerFormatPtr &log_format) {
  if (log_level == LogLevel::OFF) return;
  Logger::log_level = log_level;
  Logger::is_open_log = true;
  Logger::m_module_name = module_name;
  m_log_format = log_format;
  m_log_sinks = log_sinks;
  registerCoredumpHandler();
}

const std::vector<std::shared_ptr<logger::LogSink>> &Logger::getLogSinks() {
  return m_log_sinks;
}

void Logger::addSink(const logger::LogSink::LogSinkPtr &log_sink) {
  m_log_sinks.emplace_back(log_sink);
}

Logger *Logger::getInstance() { return logger_ptr; }

Logger::~Logger() {
  for (auto &sink : m_log_sinks) {
    sink->flush();
  }
}

/*********************************************************/
/****************SyncLogger*******************************/
/*********************************************************/

SyncLogger *SyncLogger::getInstance() {
  static SyncLogger sync_logger;
  // 仅在 logger_ptr 未被设置时赋值，避免覆盖其他子类实例
  if (logger_ptr == nullptr) {
    logger_ptr = &sync_logger;
  }
  return &sync_logger;
}

void SyncLogger::log(logger::LogEvent log_event) {
  std::lock_guard<std::mutex> lock(m_mtx);
  for (auto &sink : m_log_sinks) {
    sink->sink(log_event, m_log_format);
  }
}
/*********************************************************/
/****************AsyncLogger*******************************/
/*********************************************************/

AsyncLogger *AsyncLogger::getInstance() {
  static AsyncLogger async_logger;
  // 仅在 logger_ptr 未被设置时赋值，避免覆盖其他子类实例
  if (logger_ptr == nullptr) {
    logger_ptr = &async_logger;
  }
  return &async_logger;
}

void AsyncLogger::beforeExit() {
  m_logs.stop();
  if (m_async_thread.joinable()) m_async_thread.join();
}

AsyncLogger::~AsyncLogger() { beforeExit(); }

void logger::AsyncLogger::init(
    const logger::LogLevel &log_level, const std::string &module_name,
    const logger::LogSink::LogSinkPtr &log_sink,
    const logger::LoggerFormat::LoggerFormatPtr &log_format, int queue_size) {
  if (log_level == OFF) return;
  Logger::log_level = log_level;
  Logger::is_open_log = true;
  Logger::m_module_name = module_name;
  m_log_sinks.emplace_back(log_sink);
  m_logs.resize(queue_size);
  m_log_format = log_format;
  m_async_thread =
      std::thread(&AsyncLogger::bgLogLoop, AsyncLogger::getInstance());
  registerCoredumpHandler();
}

void logger::AsyncLogger::init(
    const logger::LogLevel &log_level, const std::string &module_name,
    const std::vector<logger::LogSink::LogSinkPtr> &log_sinks,
    const logger::LoggerFormat::LoggerFormatPtr &log_format, int queue_size) {
  if (log_level == OFF) return;
  Logger::log_level = log_level;
  Logger::is_open_log = true;
  Logger::m_module_name = module_name;
  m_log_sinks = log_sinks;
  m_logs.resize(queue_size);
  m_log_format = log_format;
  m_async_thread =
      std::thread(&AsyncLogger::bgLogLoop, AsyncLogger::getInstance());
  registerCoredumpHandler();
}

// 另起一个线程运行
void AsyncLogger::bgLogLoop() {
  while (true) {
    logger::LogEvent log_event;

    if (!m_logs.pop(log_event)) {
      // 队列停止了，退出前 flush 所有 sink
      for (auto &sink : m_log_sinks) {
        sink->flush();
      }
      break;
    }
    if (log_event.m_log_msg.empty()) continue;
    for (auto &sink : m_log_sinks) {
      sink->sink(log_event, m_log_format);
    }
    // 如果队列已经被消费空了，主动 flush 一次
    if (m_logs.empty()) {
      for (auto &sink : m_log_sinks) {
        sink->flush();
      }
    }
  }
}

void AsyncLogger::log(logger::LogEvent log_event) {
  m_logs.push(std::move(log_event));
}
};  // namespace logger