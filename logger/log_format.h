#ifndef LOGGER_LOG_FORMAT
#define LOGGER_LOG_FORMAT
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "log_event.h"
namespace logger {

class FormatItem {
 public:
  typedef std::shared_ptr<FormatItem> FormatItemPtr;
  virtual void format(std::ostream &os, const logger::LogEvent &log_msg) = 0;
  virtual void setFmt(const std::string &fmt);
  virtual std::string getFmt();

 private:
};

class DateFormatItem : public FormatItem {
 public:
  DateFormatItem(const std::string &time_fmt = "%Y-%m-%d %H-%M-%S");
  void format(std::ostream &os, const logger::LogEvent &log_msg) override;

 private:
  std::string m_time_fmt;
};
class TabFormatItem : public FormatItem {
 public:
  void format(std::ostream &os, const logger::LogEvent &log_msg) override;

 private:
};

class LevelFormatItem : public FormatItem {
 public:
  void format(std::ostream &os, const logger::LogEvent &log_msg) override;

 private:
};

class ProcessIdFormatItem : public FormatItem {
 public:
  void format(std::ostream &os, const logger::LogEvent &log_msg) override;

 private:
};

class ThreadIdFormatItem : public FormatItem {
 public:
  void format(std::ostream &os, const logger::LogEvent &log_msg) override;

 private:
};

class CoroutineIdFormatItem : public FormatItem {
 public:
  void format(std::ostream &os, const logger::LogEvent &log_msg) override;

 private:
};

class ModuleNameFormatItem : public FormatItem {
 public:
  void format(std::ostream &os, const logger::LogEvent &log_msg) override;

 private:
};

class FileNameFormatItem : public FormatItem {
 public:
  void format(std::ostream &os, const logger::LogEvent &log_msg) override;

 private:
};

class FunctionNameFormatItem : public FormatItem {
 public:
  void format(std::ostream &os, const logger::LogEvent &log_msg) override;

 private:
};

class LineIdFormatItem : public FormatItem {
 public:
  void format(std::ostream &os, const logger::LogEvent &log_msg) override;

 private:
};

class LogMsgFormatItem : public FormatItem {
 public:
  void format(std::ostream &os, const logger::LogEvent &log_msg) override;

 private:
};

class NewLineFormatItem : public FormatItem {
 public:
  void format(std::ostream &os, const logger::LogEvent &log_msg) override;

 private:
};

class OtherFormatItem : public FormatItem {
 public:
  OtherFormatItem(const std::string &other_fmt);
  void format(std::ostream &os, const logger::LogEvent &log_msg) override;
  void setFmt(const std::string &fmt) override;
  std::string getFmt() override;

 private:
  std::string m_other_fmt;
};

class LoggerFormat {
 public:
  typedef std::shared_ptr<LoggerFormat> LoggerFormatPtr;
  // %d{%Y-%m-%d %H:%M:%S} 日期格式
  // %T : \space            TabFormatItem
  // %L : 日志级别           LevelFormatItem
  // %p : 进程id             ProcessIdFormatItem
  // %t : 线程id             ThreadIdFormatItem
  // %c : 协程id             FiberIdFormatItem
  // %M : 模块名           NameFormatItem
  // %F : 文件名             FilenameFormatItem
  // %f : 函数名
  // %l : 行号               LineFormatItem
  // %m : 日志内容           MessageFormatItem
  // %n : 换行符[\r\n]       NewLineFormatItem
  LoggerFormat(
      const std::string &pattern =
          "%d{%Y-%m-%d %H:%M:%S}%T[%L]%T[%p,%t,%c]%T[%M]%T[%F:%f:%l]%T%m%n");
  ~LoggerFormat() = default;
  void parserPattern();
  void format(std::ostream &os, const logger::LogEvent &log_msg);
  void setPattern(const std::string &pattern);

 public:
  std::string m_pattern;
  std::vector<FormatItem::FormatItemPtr> m_format_items;
};

class FormatItemFactory {
 public:
  static std::shared_ptr<FormatItem> createFormatItem(
      const char &key, const std::string &val = std::string());
  static bool canCreate(const char &key);
};
};  // namespace logger

#endif