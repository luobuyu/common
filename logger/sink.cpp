#include "sink.h"

#include <filesystem>
namespace dry {
namespace logger {
logger::FileSink::FileSink(std::string file_path, uint32_t max_size,
                           dry::clock::duration rotate_interval)
    : m_file_path(std::move(file_path)),
      m_no(0),
      m_max_size(max_size),
      m_rotate_interval(rotate_interval),
      m_next_start(dry::clock::now() + m_rotate_interval) {}
void FileSink::sink(const logger::LogEvent &log_event,
                    LoggerFormat::LoggerFormatPtr &log_format) {
  bool create_new_file = !m_ofs.is_open();
  // 如果超过了日期
  if (log_event.m_timestamp >= m_next_start) {
    // 直接计算需要跳过的周期数，O(1) 定位到当前所在的周期
    // 避免程序暂停后恢复时频繁创建空文件
    auto elapsed = log_event.m_timestamp - m_next_start;
    auto periods = elapsed / m_rotate_interval + 1;
    m_next_start += periods * m_rotate_interval;
    m_no = 0;
    create_new_file = true;
  }
  // 如果需要打开新文件
  if (create_new_file) {
    openNewFile();
  }
  // 判断文件大小
  if (m_ofs.tellp() > m_max_size) {
    m_no++;
    openNewFile();
  }
  log_format->format(m_ofs, log_event);
}

void FileSink::openNewFile() {
  if (m_ofs.is_open()) {
    m_ofs.flush();
    m_ofs.close();
  }
  std::string date = dry::getTime(m_next_start - m_rotate_interval, "%Y%m%d%H");
  std::filesystem::path file_name = date + "-" + std::to_string(m_no) + ".log";
  std::filesystem::path file_full_name = m_file_path / file_name;
  // 确保日志目录存在
  std::filesystem::create_directories(m_file_path);
  m_ofs.open(file_full_name, std::ios::app);
  if (!m_ofs.is_open()) {
    std::cerr << "fail to open " << file_full_name << ", abort!" << std::endl;
    std::abort();
  }
}

void FileSink::flush() { m_ofs.flush(); }

void StdoutSink::sink(const logger::LogEvent &log_event,
                      LoggerFormat::LoggerFormatPtr &log_format) {
  log_format->format(std::cout, log_event);
}

void StdoutSink::flush() { std::cout.flush(); }
};  // namespace logger
}  // namespace dry