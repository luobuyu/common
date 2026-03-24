
#include "dry_log.h"

#include <memory>
#include <string>
#include <vector>

#include "../config/config.h"
#include "../logger/log_manager.h"
#include "../logger/sink.h"

namespace dry {

void initLogger(config::Config& conf) {
  auto& mgr = logger::LogManager::getInstance();

  std::string module_name = conf.getString("logger", "module_name", "app");
  logger::LogLevel log_level = static_cast<logger::LogLevel>(conf.getInt(
      "logger", "log_level", static_cast<int>(logger::LogLevel::INFO)));

  // 构建 Sink 列表
  std::vector<logger::LogSink::LogSinkPtr> log_sinks;
  if (conf.getInt("logger", "file_sink", 1)) {
    std::string log_path = conf.getString("logger", "log_path", "../log");
    log_sinks.emplace_back(std::make_shared<logger::FileSink>(log_path));
  }
  if (conf.getInt("logger", "std_sink", 1)) {
    log_sinks.emplace_back(std::make_shared<logger::StdoutSink>());
  }

  // 构建 Format
  auto logger_format = std::make_shared<logger::LoggerFormat>();
  std::string pattern = conf.getString("logger", "log_format", "");
  if (!pattern.empty()) {
    logger_format->setPattern(pattern);
  }

  // 支持通过配置选择同步/异步，默认异步
  auto logger_type = conf.getInt("logger", "async", 1)
                         ? logger::LogManager::LoggerType::ASYNC
                         : logger::LogManager::LoggerType::SYNC;

  mgr.init(log_level, module_name, logger_type, log_sinks, logger_format);
}

void initLogger(const std::string& module_name, int level,
                const std::string& log_path, bool enable_stdout,
                logger::LogManager::LoggerType type) {
  auto& mgr = logger::LogManager::getInstance();
  logger::LogLevel log_level = static_cast<logger::LogLevel>(level);

  std::vector<logger::LogSink::LogSinkPtr> log_sinks;
  log_sinks.emplace_back(std::make_shared<logger::FileSink>(log_path));
  if (enable_stdout) {
    log_sinks.emplace_back(std::make_shared<logger::StdoutSink>());
  }

  mgr.init(log_level, module_name, type, log_sinks);
}

void openLog(config::Config& conf) { initLogger(conf); }

}  // namespace dry
