#ifndef DRY_LOG_H
#define DRY_LOG_H
#include <string>
#include <vector>

#include "../config/config.h"
#include "../logger/log.h"

namespace dry {

/**
 * @brief 从配置文件快速初始化日志系统
 * @param conf 配置对象
 * @details 这是dry库风格的便捷函数，一行代码搞定日志初始化
 */
static void initLogger(config::Config& conf) {
  auto logger = logger::AsyncLogger::getInstance();
  std::string module_name = conf.getString("logger", "module_name", "app");
  logger::LogLevel log_level = static_cast<logger::LogLevel>(
      conf.getInt("logger", "log_level", static_cast<int>(logger::LogLevel::INFO)));
  
  std::vector<logger::LogSink::LogSinkPtr> log_sinks;
  if (conf.getInt("logger", "file_sink", 1)) {
    std::string log_path = conf.getString("logger", "log_path", "../log");
    log_sinks.emplace_back(new logger::FileSink(log_path));
  }
  if (conf.getInt("logger", "std_sink", 1)) {
    log_sinks.emplace_back(new logger::StdoutSink());
  }
  
  auto logger_format = logger::LoggerFormat::LoggerFormatPtr(new logger::LoggerFormat());
  std::string pattern = conf.getString("logger", "log_format", "");
  if (!pattern.empty()) {
    logger_format->setPattern(pattern);
  }
  
  logger->init(log_level, module_name, log_sinks, logger_format);
}

/**
 * @brief 快速初始化日志系统（不需要配置文件）
 * @param module_name 模块名称
 * @param level 日志级别 (1=DEBUG, 2=INFO, 3=WARN, 4=ERROR, 5=OFF)
 * @param log_path 日志文件路径
 * @param enable_stdout 是否输出到控制台
 */
static void initLogger(const std::string& module_name, 
                      int level = 2, 
                      const std::string& log_path = "../log",
                      bool enable_stdout = true) {
  auto logger = logger::AsyncLogger::getInstance();
  logger::LogLevel log_level = static_cast<logger::LogLevel>(level);
  
  std::vector<logger::LogSink::LogSinkPtr> log_sinks;
  log_sinks.emplace_back(new logger::FileSink(log_path));
  if (enable_stdout) {
    log_sinks.emplace_back(new logger::StdoutSink());
  }
  
  logger->init(log_level, module_name, log_sinks);
}

/**
 * @brief 关闭日志系统
 */
static void shutdownLogger() {
  if (auto logger = logger::AsyncLogger::getInstance()) {
    logger->shutDownNow();
  }
}

// 向后兼容的函数名
static void openLog(config::Config& conf) {
  initLogger(conf);
}
};  // namespace dry

#endif