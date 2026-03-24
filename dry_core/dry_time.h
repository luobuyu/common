#ifndef DRY_TIME_H
#define DRY_TIME_H
#include <chrono>
#include <ctime>
#include <string>
namespace dry {
using clock = std::chrono::system_clock;

inline std::string getTime(std::string format) {
  auto now = clock::now();
  // 获取当前时间
  std::time_t t = clock::to_time_t(now);

  // 将时间转换为本地时间（线程安全）
  std::tm local_time;
  localtime_r(&t, &local_time);

  // 创建缓冲区来存储格式化日期
  char date_buffer[32];

  // 格式化输出日期（格式：YYYY-MM-DD）
  std::strftime(date_buffer, sizeof(date_buffer), format.c_str(), &local_time);
  return std::string(date_buffer);
}

inline std::string getTime(const clock::time_point& t,
                           const std::string& format) {
  // 将时间转换为本地时间（线程安全）
  std::time_t t_time_t = clock::to_time_t(t);
  std::tm local_time;
  localtime_r(&t_time_t, &local_time);

  // 创建缓冲区来存储格式化日期
  char date_buffer[32];

  // 格式化输出日期
  std::strftime(date_buffer, sizeof(date_buffer), format.c_str(), &local_time);
  return std::string(date_buffer);
}

// 带毫秒的时间格式化，在 getTime 结果后追加 .毫秒
inline std::string getTimeWithMs(const clock::time_point& t,
                                 const std::string& format) {
  std::string result = getTime(t, format);
  auto epoch = t.time_since_epoch();
  auto millis =
      std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count() %
      1000;
  char ms_buf[8];
  snprintf(ms_buf, sizeof(ms_buf), ".%03d", static_cast<int>(millis));
  return result + ms_buf;
}

class Timer {
 private:
  using TimePoint = clock::time_point;
  TimePoint startTime, endTime;
  long long timeout;

 public:
  Timer() : startTime(clock::now()) {}
  Timer(long long secTimeout) {
    timeout = secTimeout;
    setTimer(secTimeout);
  }
  void setTimer(long long secTimeout) {
    startTime = clock::now();
    endTime = startTime + std::chrono::seconds(secTimeout);
  }
  bool isTimeout() { return endTime < clock::now(); }
  std::chrono::duration<double> getDuration() {
    return clock::now() - startTime;
  }
  double getDurationS() {
    return std::chrono::duration<double>(getDuration()).count();
  }
  double getDurationMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(getDuration())
        .count();
  }
  double getDurationUs() {
    return std::chrono::duration_cast<std::chrono::microseconds>(getDuration())
        .count();
  }

  long long getTimeout() { return timeout; }
};

}  // namespace dry
#endif