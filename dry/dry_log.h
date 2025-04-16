#ifndef DRY_LOG_H
#define DRY_LOG_H
#include <string>
#include <vector>

#include "config/config.h"
#include "logger/log.h"
namespace dry {
// 定义一些宏定义简化日志操作

static void openLog(config::Config& conf) {
  auto logger = logger::AsyncLogger::getInstance();
  std::string module_name = conf.getString("logger", "module_name");
  logger::LogLevel log_level = static_cast<logger::LogLevel>(
      conf.getInt("logger", "log_level", logger::LogLevel::OFF));
  std::vector<logger::LogSink::LogSinkPtr> log_sinks;
  if (conf.getInt("logger", "file_sink")) {
    std::string log_path = conf.getString("logger", "log_path", "../log");
    log_sinks.emplace_back(new logger::FileSink(log_path));
  }
  if (conf.getInt("logger", "std_sink")) {
    log_sinks.emplace_back(new logger::StdoutSink());
  }
  auto logger_format =
      logger::LoggerFormat::LoggerFormatPtr(new logger::LoggerFormat());
  std::string pattern = conf.getString("logger", "log_format");
  if (!pattern.empty()) {
    logger_format->setPattern(pattern);
  }
  logger->init(log_level, module_name, log_sinks, logger_format);
}
};  // namespace dry

#endif