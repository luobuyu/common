#ifndef DRY_LOG_H
#define DRY_LOG_H

/**
 * @file dry_log.h
 * @brief 日志系统用户入口 — 初始化函数 + 便捷宏
 * @details 使用者只需 #include 此文件即可使用
 * LOG_DEBUG/LOG_INFO/LOG_WARN/LOG_ERROR 宏。 初始化可选：通过 Config
 * 对象或手动指定参数。
 */

#include <string>

#include "../logger/log_manager.h"

// 前向声明，避免在头文件中 include config.h（只有 .cpp 中需要）
namespace dry {
namespace config {
class Config;
}  // namespace config
}  // namespace dry

namespace dry {

/**
 * @brief 从配置文件快速初始化日志系统
 * @param conf 配置对象
 * @details 一行代码搞定日志初始化。支持通过配置项 logger.async
 * 选择同步/异步，默认异步。
 */
void initLogger(config::Config& conf);

/**
 * @brief 快速初始化日志系统（不需要配置文件）
 * @param module_name 模块名称
 * @param level 日志级别，对应 logger::LogLevel 枚举（DEBUG=1, INFO=2, WARN=3,
 *               ERROR=4, OFF=5）
 * @param log_path 日志文件路径
 * @param enable_stdout 是否输出到控制台
 * @param type 日志器类型，默认异步
 */
void initLogger(const std::string& module_name, int level = 2,
                const std::string& log_path = "../log",
                bool enable_stdout = true,
                logger::LogManager::LoggerType type =
                    logger::LogManager::LoggerType::ASYNC);

/// 向后兼容的函数名
void openLog(config::Config& conf);

}  // namespace dry

// =============================================================================
// 便捷的日志宏定义
// =============================================================================

/**
 * @internal 内部实现宏，请勿直接调用，应使用
 * LOG_DEBUG/LOG_INFO/LOG_WARN/LOG_ERROR
 */
#define LOG_FMT(level, fmt, ...)                                         \
  do {                                                                   \
    auto& _log_mgr = dry::logger::LogManager::getInstance();             \
    if (_log_mgr.isOpen() && _log_mgr.shouldLog(level)) {                \
      _log_mgr.getLogger()->log(dry::logger::LogEvent(                   \
          level, _log_mgr.getModuleName(), __FILE__, __func__, __LINE__, \
          dry::logger::formatString(fmt, ##__VA_ARGS__)));               \
    }                                                                    \
  } while (0)

/// @name 日志输出宏
/// @brief 格式化输出日志，语法兼容 printf
/// @code
///   LOG_DEBUG("user_id=%d, name=%s", uid, name.c_str());
///   LOG_INFO("server started on port %d", port);
///   LOG_WARN("retry count exceeded: %d", retry);
///   LOG_ERROR("connect failed, ret=%d", ret);
/// @endcode
/// @{
#define LOG_DEBUG(fmt, ...) \
  LOG_FMT(dry::logger::LogLevel::DEBUG, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) \
  LOG_FMT(dry::logger::LogLevel::INFO, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) \
  LOG_FMT(dry::logger::LogLevel::WARN, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) \
  LOG_FMT(dry::logger::LogLevel::ERROR, fmt, ##__VA_ARGS__)
/// @}

#endif