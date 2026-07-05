
#include "dry_log.h"

#include <memory>
#include <string>
#include <vector>

#include "../config/config.h"
#include "../logger/log_manager.h"
#include "../logger/sink.h"

namespace dry {

void InitLogger(config::Config& conf) {
  auto& mgr = logger::LogManager::GetInstance();

  std::string module_name = conf.GetString("logger", "module_name", "app");
  logger::LogLevel log_level = static_cast<logger::LogLevel>(
      conf.GetInt("logger", "log_level", static_cast<int>(logger::LogLevel::INFO)));

  // 构建 Sink 列表
  std::vector<logger::LogSink::LogSinkPtr> log_sinks;
  if (conf.GetInt("logger", "file_sink", 1)) {
    std::string log_path = conf.GetString("logger", "log_path", "../Log");
    log_sinks.emplace_back(std::make_shared<logger::FileSink>(log_path));
  }
  if (conf.GetInt("logger", "std_sink", 1)) {
    log_sinks.emplace_back(std::make_shared<logger::StdoutSink>());
  }

  // 构建 format
  auto logger_format = std::make_shared<logger::LoggerFormat>();
  std::string pattern = conf.GetString("logger", "log_format", "");
  if (!pattern.empty()) {
    logger_format->SetPattern(pattern);
  }

  // 支持通过配置选择同步/异步，默认异步
  auto logger_type = conf.GetInt("logger", "async", 1) ? logger::LogManager::LoggerType::Async
                                                       : logger::LogManager::LoggerType::Sync;

  mgr.Init(log_level, module_name, logger_type, log_sinks, logger_format);
}

void InitLogger(const std::string& module_name, int level, const std::string& log_path,
                bool enable_stdout, logger::LogManager::LoggerType type) {
  auto& mgr = logger::LogManager::GetInstance();
  logger::LogLevel log_level = static_cast<logger::LogLevel>(level);

  std::vector<logger::LogSink::LogSinkPtr> log_sinks;
  log_sinks.emplace_back(std::make_shared<logger::FileSink>(log_path));
  if (enable_stdout) {
    log_sinks.emplace_back(std::make_shared<logger::StdoutSink>());
  }

  auto logger_format = std::make_shared<logger::LoggerFormat>();
  mgr.Init(log_level, module_name, type, log_sinks, logger_format);
}

void OpenLog(config::Config& conf) {
  InitLogger(conf);
}

}  // namespace dry
