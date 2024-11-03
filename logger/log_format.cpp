#include "log_format.h"

namespace logger
{

    logger::LoggerFormat::LoggerFormat(const std::string &pattern): m_pattern(pattern){
        parserPattern();
    }
    void logger::FormatItem::setFmt(const std::string &fmt)
    {
    }
    std::string logger::FormatItem::getFmt()
    {
        return std::string();
    }
    void logger::LoggerFormat::parserPattern()
    {
        bool is_pre_other = false;
        for (int i = 0; i < m_pattern.size();)
        {
            if(m_pattern[i] == '%') {
                if(i + 1 < m_pattern.size() && FormatItemFactory::canCreate(m_pattern[i + 1])) {
                    if(m_pattern[i + 1] == 'd') {
                        if(i + 2 < m_pattern.size() && m_pattern[i + 2] == '{') {
                            size_t index1 = i + 2;
                            size_t index2 = m_pattern.find_first_of('}', i + 2);
                            if (index2 == m_pattern.npos)
                            {
                                std::cout << "log format error" << std::endl;
                                return;
                            }
                            std::string val = m_pattern.substr(index1 + 1, index2 - 1 - index1);
                            m_format_items.emplace_back(FormatItemFactory::createFormatItem(m_pattern[i + 1], val));
                            i = index2 + 1;
                        }
                        else
                        {
                            m_format_items.emplace_back(FormatItemFactory::createFormatItem(m_pattern[i + 1], std::string()));
                            i = i + 2;
                        }
                    } else 
                    {
                        m_format_items.emplace_back(FormatItemFactory::createFormatItem(m_pattern[i + 1]));
                        i = i + 2;
                    }
                    is_pre_other = false;
                }
                else
                {
                    // % 是末尾，或者 不能生产，就是 other
                    size_t index = m_pattern.find_first_of('%', i + 1) - 1;
                    std::string val = m_pattern.substr(i, index - i + 1);
                    if(is_pre_other)
                        m_format_items.back()->setFmt(std::move(m_format_items.back()->getFmt() + val));
                    else m_format_items.emplace_back(FormatItemFactory::createFormatItem('%', val));
                    i = index + 1;
                    is_pre_other = true;
                }
            } 
            else 
            {
                // other char
                size_t index = m_pattern.find_first_of('%', i) - 1;
                std::string val = m_pattern.substr(i, index - i + 1);
                if (is_pre_other)
                    m_format_items.back()->setFmt(std::move(m_format_items.back()->getFmt() + val));
                else
                    m_format_items.emplace_back(FormatItemFactory::createFormatItem('%', val));
                i = index + 1;
                is_pre_other = true;
            }
        }
    }
    void LoggerFormat::format(std::ostream &os, const logger::LogEvent &log_msg)
    {
        for(auto& fmt: m_format_items) {
            fmt->format(os, log_msg);
        }
    }
    void LoggerFormat::setPattern(const std::string &pattern)
    {
        m_format_items.clear();
        m_pattern = pattern;
        parserPattern();
    }
    logger::DateFormatItem::DateFormatItem(const std::string &time_fmt) : m_time_fmt(time_fmt)
    {
    }
    void DateFormatItem::format(std::ostream& os, const logger::LogEvent &log_msg)
    {
        os << dry::getTime(log_msg.m_timestamp, m_time_fmt);
    }
    void TabFormatItem::format(std::ostream& os, const logger::LogEvent &log_msg)
    {
        os << " ";
    }
    void logger::LevelFormatItem::format(std::ostream& os, const logger::LogEvent &log_msg)
    {
        os << levelToString(log_msg.m_log_level);
    }
    void ProcessIdFormatItem::format(std::ostream& os, const logger::LogEvent &log_msg)
    {
        os << log_msg.m_process_id;
    }
    void ThreadIdFormatItem::format(std::ostream& os, const logger::LogEvent &log_msg)
    {
        os << log_msg.m_thread_id;
    }
    void CoroutineIdFormatItem::format(std::ostream& os, const logger::LogEvent &log_msg)
    {
        os << log_msg.m_coroutine_id;
    }
    void ModuleNameFormatItem::format(std::ostream& os, const logger::LogEvent &log_msg)
    {
        os << log_msg.m_module_name;
    }
    void FileNameFormatItem::format(std::ostream& os, const logger::LogEvent &log_msg)
    {
        os << log_msg.m_file_name;
    }
    void FunctionNameFormatItem::format(std::ostream& os, const logger::LogEvent &log_msg)
    {
        os << log_msg.m_funtion_name;
    }
    void LineIdFormatItem::format(std::ostream& os, const logger::LogEvent &log_msg)
    {
        os << log_msg.m_line_id;
    }
    void LogMsgFormatItem::format(std::ostream& os, const logger::LogEvent &log_msg)
    {
        os << log_msg.m_log_msg;
    }
    void NewLineFormatItem::format(std::ostream& os, const logger::LogEvent &log_msg)
    {
        os << "\n";
    }
    logger::OtherFormatItem::OtherFormatItem(const std::string &other_fmt): m_other_fmt(other_fmt)
    {
    }
    void OtherFormatItem::format(std::ostream& os, const logger::LogEvent &log_msg)
    {
        os << m_other_fmt;
    }
    void logger::OtherFormatItem::setFmt(const std::string &fmt)
    {
        m_other_fmt = fmt;
    }
    std::string logger::OtherFormatItem::getFmt()
    {
        return m_other_fmt;
    }
    std::shared_ptr<FormatItem> logger::FormatItemFactory::createFormatItem(const char& key, const std::string &val)
    {
        // "%d", "%T", "%L", "%p", "%t", "%c", "%M", "%F", "%f", "%l", "%m", "%n"
        if(key == 'd')
            if(val.empty()) return FormatItem::FormatItemPtr(new DateFormatItem());
            else
                return FormatItem::FormatItemPtr(new DateFormatItem(val));
        if(key == 'T')
            return FormatItem::FormatItemPtr(new TabFormatItem());
        if (key == 'L')
            return FormatItem::FormatItemPtr(new LevelFormatItem());
        if (key == 'p')
            return FormatItem::FormatItemPtr(new ProcessIdFormatItem());
        if (key == 't')
            return FormatItem::FormatItemPtr(new ThreadIdFormatItem());
        if (key == 'c')
            return FormatItem::FormatItemPtr(new CoroutineIdFormatItem());
        if (key == 'M')
            return FormatItem::FormatItemPtr(new ModuleNameFormatItem());
        if (key == 'F')
            return FormatItem::FormatItemPtr(new FileNameFormatItem());
        if (key == 'f')
            return FormatItem::FormatItemPtr(new FunctionNameFormatItem());
        if (key == 'l')
            return FormatItem::FormatItemPtr(new LineIdFormatItem());
        if (key == 'm')
            return FormatItem::FormatItemPtr(new LogMsgFormatItem());
        if (key == 'n')
            return FormatItem::FormatItemPtr(new NewLineFormatItem());
        return FormatItem::FormatItemPtr(new OtherFormatItem(val));
    }
    bool logger::FormatItemFactory::canCreate(const char &key)
    {
        static std::unordered_set<char> keys = {'d', 'T', 'L', 'p', 't', 'c', 'M', 'F', 'f', 'l', 'm', 'n'};
        return keys.find(key) != keys.end();
    }
};