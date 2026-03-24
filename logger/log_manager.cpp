
#include "log_manager.h"

#include <csignal>
#include <iostream>

namespace logger {

LogManager& LogManager::getInstance() {
  static LogManager instance;
  return instance;
}

void LogManager::init(LogLevel log_level, std::string module_name,
                      LoggerType type, LogSink::LogSinkPtr log_sink,
                      LoggerFormat::LoggerFormatPtr log_format, int queue_size,
                      std::chrono::milliseconds flush_interval) {
  std::vector<LogSink::LogSinkPtr> sinks = {std::move(log_sink)};
  init(log_level, std::move(module_name), type, std::move(sinks),
       std::move(log_format), queue_size, flush_interval);
}

void LogManager::init(LogLevel log_level, std::string module_name,
                      LoggerType type,
                      std::vector<LogSink::LogSinkPtr> log_sinks,
                      LoggerFormat::LoggerFormatPtr log_format, int queue_size,
                      std::chrono::milliseconds flush_interval) {
  if (log_level == LogLevel::OFF) return;
  std::call_once(m_init_flag, [&]() {
    m_log_level = log_level;
    m_module_name = std::move(module_name);

    // 根据类型创建对应的 Logger
    if (type == LoggerType::ASYNC) {
      m_logger = std::make_unique<AsyncLogger>(std::move(log_sinks),
                                               std::move(log_format),
                                               queue_size, flush_interval);
    } else {
      m_logger = std::make_unique<SyncLogger>(std::move(log_sinks),
                                              std::move(log_format));
    }

    m_is_open = true;
    registerCoredumpHandler();
  });
}

bool LogManager::isOpen() const { return m_is_open && m_logger != nullptr; }

bool LogManager::shouldLog(LogLevel level) const {
  return level <= m_log_level;
}

const std::string& LogManager::getModuleName() const { return m_module_name; }

Logger* LogManager::getLogger() const { return m_logger.get(); }

void LogManager::addSink(const LogSink::LogSinkPtr& log_sink) {
  if (m_logger) {
    m_logger->addSink(log_sink);
  }
}

void LogManager::registerCoredumpHandler() {
#if defined(__linux__) || defined(__APPLE__)
  signal(SIGSEGV, coredumpHandler);
  signal(SIGABRT, coredumpHandler);
  signal(SIGTERM, coredumpHandler);
  // SIGKILL 不可捕获，不注册
  signal(SIGINT, coredumpHandler);
  signal(SIGHUP, coredumpHandler);
  // 忽略 SIGPIPE
  signal(SIGPIPE, SIG_IGN);
#endif
#if defined(__linux__)
  signal(SIGSTKFLT, coredumpHandler);
#endif
}

void LogManager::coredumpHandler(int signal_no) {
  std::cerr << "process received invalid signal, will exit" << std::endl;
  // 尽量 flush 已有日志到磁盘
  auto* logger = LogManager::getInstance().getLogger();
  if (logger != nullptr) {
    for (auto& sink : logger->getLogSinks()) {
      sink->flush();
    }
  }
  signal(signal_no, SIG_DFL);
  raise(signal_no);
}

LogManager::~LogManager() {
  if (m_logger) {
    m_logger->beforeExit();
  }
}

}  // namespace logger
