#include "log.h"

namespace dry {
namespace logger {

// =============================================================================
// Logger 基类实现
// =============================================================================

Logger::Logger(std::vector<LogSink::LogSinkPtr> sinks,
               LoggerFormat::LoggerFormatPtr fmt)
    : m_log_sinks(std::move(sinks)), m_log_format(std::move(fmt)) {}

void Logger::addSink(const LogSink::LogSinkPtr &log_sink) {
  std::lock_guard<std::mutex> lock(m_sink_mtx);
  m_log_sinks.emplace_back(log_sink);
}

const std::vector<LogSink::LogSinkPtr> &Logger::getLogSinks() const {
  return m_log_sinks;
}

Logger::~Logger() {
  for (auto &sink : m_log_sinks) {
    sink->flush();
  }
}

// =============================================================================
// SyncLogger 实现
// =============================================================================

SyncLogger::SyncLogger(std::vector<LogSink::LogSinkPtr> sinks,
                       LoggerFormat::LoggerFormatPtr fmt)
    : Logger(std::move(sinks), std::move(fmt)) {}

void SyncLogger::log(LogEvent log_event) {
  std::lock_guard<std::mutex> lock(m_mtx);
  for (auto &sink : m_log_sinks) {
    sink->sink(log_event, m_log_format);
  }
}

// =============================================================================
// AsyncLogger 实现
// =============================================================================

AsyncLogger::AsyncLogger(std::vector<LogSink::LogSinkPtr> sinks,
                         LoggerFormat::LoggerFormatPtr fmt, int queue_size,
                         std::chrono::milliseconds flush_interval)
    : Logger(std::move(sinks), std::move(fmt)) {
  startBgThread(queue_size, flush_interval);
}

void AsyncLogger::startBgThread(int queue_size,
                                std::chrono::milliseconds flush_interval) {
  m_logs.resize(queue_size);
  m_flush_interval = flush_interval;
  m_async_thread = std::thread(&AsyncLogger::bgLogLoop, this);
}

void AsyncLogger::beforeExit() {
  m_logs.stop();
  if (m_async_thread.joinable()) m_async_thread.join();
}

AsyncLogger::~AsyncLogger() { beforeExit(); }

void AsyncLogger::log(LogEvent log_event) { m_logs.push(std::move(log_event)); }

void AsyncLogger::bgLogLoop() {
  static constexpr std::size_t kBatchSize = 64;  // 每批最多弹出条数
  std::vector<LogEvent> batch;
  batch.reserve(kBatchSize);

  while (true) {
    batch.clear();
    std::size_t n =
        m_logs.batchPopWithTimeout(batch, kBatchSize, m_flush_interval);

    if (n > 0) {
      // 批量写入所有 sink
      for (auto &log_event : batch) {
        if (!log_event.m_log_msg.empty()) {
          for (auto &sink : m_log_sinks) {
            sink->sink(log_event, m_log_format);
          }
        }
      }
    } else {
      // 超时或 stop+空 → flush 一次确保已有日志落盘
      for (auto &sink : m_log_sinks) {
        sink->flush();
      }
      if (m_logs.isStopping()) {
        break;
      }
    }
  }
}

}  // namespace logger
}  // namespace dry