#include "log_format.h"

#include <unordered_set>
namespace dry {
namespace logger {

logger::LoggerFormat::LoggerFormat(std::string pattern) : m_pattern(std::move(pattern)) {
  ParsePattern();
}
void logger::FormatItem::SetFmt(const std::string &fmt) {}
std::string logger::FormatItem::GetFmt() {
  return std::string();
}
void logger::LoggerFormat::ParsePattern() {
  bool is_pre_other = false;
  for (int i = 0; i < m_pattern.size();) {
    if (m_pattern[i] == '%') {
      if (i + 1 < m_pattern.size() && FormatItemFactory::CanCreate(m_pattern[i + 1])) {
        if (m_pattern[i + 1] == 'd') {
          if (i + 2 < m_pattern.size() && m_pattern[i + 2] == '{') {
            size_t index1 = i + 2;
            size_t index2 = m_pattern.find_first_of('}', i + 2);
            if (index2 == m_pattern.npos) {
              std::cout << "Log format Error" << std::endl;
              return;
            }
            std::string val = m_pattern.substr(index1 + 1, index2 - 1 - index1);
            m_format_items.emplace_back(FormatItemFactory::CreateFormatItem(m_pattern[i + 1], val));
            i = index2 + 1;
          } else {
            m_format_items.emplace_back(
                FormatItemFactory::CreateFormatItem(m_pattern[i + 1], std::string()));
            i = i + 2;
          }
        } else {
          m_format_items.emplace_back(FormatItemFactory::CreateFormatItem(m_pattern[i + 1]));
          i = i + 2;
        }
        is_pre_other = false;
      } else {
        // % 是末尾，或者 不能生产，就是 other
        size_t pos = m_pattern.find_first_of('%', i + 1);
        size_t index = (pos == std::string::npos) ? m_pattern.size() - 1 : pos - 1;
        std::string val = m_pattern.substr(i, index - i + 1);
        if (is_pre_other) {
          m_format_items.back()->SetFmt(std::move(m_format_items.back()->GetFmt() + val));
        } else {
          m_format_items.emplace_back(FormatItemFactory::CreateFormatItem('%', val));
        }
        i = index + 1;
        is_pre_other = true;
      }
    } else {
      // other char
      size_t pos = m_pattern.find_first_of('%', i);
      size_t index = (pos == std::string::npos) ? m_pattern.size() - 1 : pos - 1;
      std::string val = m_pattern.substr(i, index - i + 1);
      if (is_pre_other) {
        m_format_items.back()->SetFmt(std::move(m_format_items.back()->GetFmt() + val));
      } else {
        m_format_items.emplace_back(FormatItemFactory::CreateFormatItem('%', val));
      }
      i = index + 1;
      is_pre_other = true;
    }
  }
}
void LoggerFormat::Format(std::ostream &os, const logger::LogEvent &log_msg) {
  for (auto &fmt : m_format_items) {
    fmt->Format(os, log_msg);
  }
}
void LoggerFormat::SetPattern(const std::string &pattern) {
  m_format_items.clear();
  m_pattern = pattern;
  ParsePattern();
}
logger::DateFormatItem::DateFormatItem(std::string time_fmt) : m_time_fmt(std::move(time_fmt)) {}
void DateFormatItem::Format(std::ostream &os, const logger::LogEvent &log_msg) {
  os << dry::GetTimeWithMs(log_msg.timestamp, m_time_fmt);
}
void TabFormatItem::Format(std::ostream &os, const logger::LogEvent &log_msg) {
  os << " ";
}
void logger::LevelFormatItem::Format(std::ostream &os, const logger::LogEvent &log_msg) {
  os << LevelToString(log_msg.log_level);
}
void ProcessIdFormatItem::Format(std::ostream &os, const logger::LogEvent &log_msg) {
  os << log_msg.process_id;
}
void ThreadIdFormatItem::Format(std::ostream &os, const logger::LogEvent &log_msg) {
  os << log_msg.thread_id;
}
void CoroutineIdFormatItem::Format(std::ostream &os, const logger::LogEvent &log_msg) {
  os << log_msg.coroutine_id;
}
void ModuleNameFormatItem::Format(std::ostream &os, const logger::LogEvent &log_msg) {
  os << log_msg.module_name;
}
void FileNameFormatItem::Format(std::ostream &os, const logger::LogEvent &log_msg) {
  os << log_msg.file_name;
}
void FunctionNameFormatItem::Format(std::ostream &os, const logger::LogEvent &log_msg) {
  os << log_msg.function_name;
}
void LineIdFormatItem::Format(std::ostream &os, const logger::LogEvent &log_msg) {
  os << log_msg.line_id;
}
void LogMsgFormatItem::Format(std::ostream &os, const logger::LogEvent &log_msg) {
  os << log_msg.log_msg;
}
void NewLineFormatItem::Format(std::ostream &os, const logger::LogEvent &log_msg) {
  os << "\n";
}
logger::OtherFormatItem::OtherFormatItem(std::string other_fmt)
    : m_other_fmt(std::move(other_fmt)) {}
void OtherFormatItem::Format(std::ostream &os, const logger::LogEvent &log_msg) {
  os << m_other_fmt;
}
void logger::OtherFormatItem::SetFmt(const std::string &fmt) {
  m_other_fmt = fmt;
}
std::string logger::OtherFormatItem::GetFmt() {
  return m_other_fmt;
}
std::shared_ptr<FormatItem> logger::FormatItemFactory::CreateFormatItem(const char &key,
                                                                        const std::string &val) {
  // "%d", "%T", "%L", "%p", "%t", "%c", "%M", "%F", "%f", "%l", "%m", "%n"
  if (key == 'd') {
    if (val.empty()) {
      return std::make_shared<DateFormatItem>();
    } else {
      return std::make_shared<DateFormatItem>(val);
    }
  }
  if (key == 'T') {
    return std::make_shared<TabFormatItem>();
  }
  if (key == 'L') {
    return std::make_shared<LevelFormatItem>();
  }
  if (key == 'p') {
    return std::make_shared<ProcessIdFormatItem>();
  }
  if (key == 't') {
    return std::make_shared<ThreadIdFormatItem>();
  }
  if (key == 'c') {
    return std::make_shared<CoroutineIdFormatItem>();
  }
  if (key == 'M') {
    return std::make_shared<ModuleNameFormatItem>();
  }
  if (key == 'F') {
    return std::make_shared<FileNameFormatItem>();
  }
  if (key == 'f') {
    return std::make_shared<FunctionNameFormatItem>();
  }
  if (key == 'l') {
    return std::make_shared<LineIdFormatItem>();
  }
  if (key == 'm') {
    return std::make_shared<LogMsgFormatItem>();
  }
  if (key == 'n') {
    return std::make_shared<NewLineFormatItem>();
  }
  return std::make_shared<OtherFormatItem>(val);
}
bool logger::FormatItemFactory::CanCreate(const char &key) {
  static std::unordered_set<char> keys = {'d', 'T', 'L', 'p', 't', 'c',
                                          'M', 'F', 'f', 'l', 'm', 'n'};
  return keys.find(key) != keys.end();
}
};  // namespace logger
}  // namespace dry