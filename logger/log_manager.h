
#ifndef LOGGER_LOG_MANAGER_H
#define LOGGER_LOG_MANAGER_H

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "log.h"

namespace dry {
namespace logger {

/**
 * @brief 日志管理器（唯一单例）
 * @details 负责日志系统的全局状态管理、Logger 实例的创建与持有、信号处理注册。
 *          所有外部代码应通过 LogManager 访问日志功能，而非直接操作 Logger
 * 子类。
 */
class LogManager {
 public:
  /// 日志器类型枚举
  enum class LoggerType {
    SYNC,   ///< 同步日志器
    ASYNC,  ///< 异步日志器
  };

  /// 获取单例引用（Meyers' Singleton）
  static LogManager& getInstance();

  // 禁止拷贝和赋值
  LogManager(const LogManager&) = delete;
  LogManager& operator=(const LogManager&) = delete;

  /**
   * @brief 初始化日志系统（单 sink 版本）
   * @param log_level 日志级别
   * @param module_name 模块名称
   * @param type 日志器类型（同步/异步）
   * @param log_sink 日志输出目标
   * @param log_format 日志格式化器
   * @param queue_size 异步队列大小（仅异步模式有效）
   * @param flush_interval 异步刷盘间隔（仅异步模式有效）
   */
  void init(LogLevel log_level, std::string module_name, LoggerType type,
            LogSink::LogSinkPtr log_sink,
            LoggerFormat::LoggerFormatPtr log_format =
                std::make_shared<LoggerFormat>(),
            int queue_size = 8192,
            std::chrono::milliseconds flush_interval =
                std::chrono::milliseconds(3000));

  /**
   * @brief 初始化日志系统（多 sink 版本）
   * @param log_level 日志级别
   * @param module_name 模块名称
   * @param type 日志器类型（同步/异步）
   * @param log_sinks 日志输出目标列表
   * @param log_format 日志格式化器
   * @param queue_size 异步队列大小（仅异步模式有效）
   * @param flush_interval 异步刷盘间隔（仅异步模式有效）
   */
  void init(LogLevel log_level, std::string module_name, LoggerType type,
            std::vector<LogSink::LogSinkPtr> log_sinks,
            LoggerFormat::LoggerFormatPtr log_format =
                std::make_shared<LoggerFormat>(),
            int queue_size = 8192,
            std::chrono::milliseconds flush_interval =
                std::chrono::milliseconds(3000));

  /// 日志系统是否已初始化并开启
  bool isOpen() const;

  /// 当前日志级别是否应该输出
  bool shouldLog(LogLevel level) const;

  /// 获取模块名称
  const std::string& getModuleName() const;

  /// 获取当前 Logger 实例（可能为 nullptr，未初始化时）
  Logger* getLogger() const;

  /// 动态添加 sink
  void addSink(const LogSink::LogSinkPtr& log_sink);

 private:
  LogManager() = default;
  ~LogManager();

  /// 注册 coredump 信号处理函数
  void registerCoredumpHandler();
  /// 信号处理回调
  static void coredumpHandler(int signal_no);

  std::once_flag m_init_flag;            ///< 保证 init 只执行一次
  std::unique_ptr<Logger> m_logger;      ///< 持有的日志器实例
  LogLevel m_log_level = LogLevel::OFF;  ///< 全局日志级别
  std::string m_module_name;             ///< 模块名称
  bool m_is_open = false;                ///< 日志是否已开启
};

}  // namespace logger
}  // namespace dry

#endif
