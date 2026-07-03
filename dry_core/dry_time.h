#ifndef DRY_TIME_H
#define DRY_TIME_H
#include <chrono>
#include <cstdint>
#include <ctime>
#include <string>
namespace dry {

// 获取单调时钟的当前时间戳，不受系统时间修改影响，用于定时器等场景
// 默认返回毫秒，可通过模板参数指定精度，如 GetNow<std::chrono::microseconds>()
template <typename Duration = std::chrono::milliseconds>
inline int64_t GetNow() {
  return std::chrono::duration_cast<Duration>(
             std::chrono::steady_clock::now().time_since_epoch())
      .count();
}

// 常用精度的简洁别名
inline int64_t GetNowS() { return GetNow<std::chrono::seconds>(); }
inline int64_t GetNowMs() { return GetNow<std::chrono::milliseconds>(); }
inline int64_t GetNowUs() { return GetNow<std::chrono::microseconds>(); }

inline std::string GetTime(std::string format) {
  auto now = std::chrono::system_clock::now();
  // 获取当前时间
  std::time_t t = std::chrono::system_clock::to_time_t(now);

  // 将时间转换为本地时间（线程安全）
  std::tm local_time;
  localtime_r(&t, &local_time);

  // 创建缓冲区来存储格式化日期
  char date_buffer[32];

  // 格式化输出日期（格式：YYYY-MM-DD）
  std::strftime(date_buffer, sizeof(date_buffer), format.c_str(), &local_time);
  return std::string(date_buffer);
}

inline std::string GetTime(const std::chrono::system_clock::time_point& t,
                           const std::string& format) {
  // 将时间转换为本地时间（线程安全）
  std::time_t t_time_t = std::chrono::system_clock::to_time_t(t);
  std::tm local_time;
  localtime_r(&t_time_t, &local_time);

  // 创建缓冲区来存储格式化日期
  char date_buffer[32];

  // 格式化输出日期
  std::strftime(date_buffer, sizeof(date_buffer), format.c_str(), &local_time);
  return std::string(date_buffer);
}

// 带毫秒的时间格式化，在 GetTime 结果后追加 .毫秒
inline std::string GetTimeWithMs(const std::chrono::system_clock::time_point& t,
                                 const std::string& format) {
  std::string Result = GetTime(t, format);
  auto epoch = t.time_since_epoch();
  auto millis =
      std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count() %
      1000;
  char ms_buf[8];
  snprintf(ms_buf, sizeof(ms_buf), ".%03d", static_cast<int>(millis));
  return Result + ms_buf;
}

class Timer {
 private:
  using SteadyClock = std::chrono::steady_clock;
  using TimePoint = SteadyClock::time_point;
  TimePoint StartTime, EndTime;
  long long timeout;

 public:
  Timer() : StartTime(SteadyClock::now()), EndTime(StartTime), timeout(0) {}
  Timer(long long secTimeout) {
    timeout = secTimeout;
    SetTimer(secTimeout);
  }
  void SetTimer(long long secTimeout) {
    StartTime = SteadyClock::now();
    EndTime = StartTime + std::chrono::seconds(secTimeout);
  }
  bool IsTimeout() { return EndTime < SteadyClock::now(); }
  std::chrono::duration<double> GetDuration() {
    return SteadyClock::now() - StartTime;
  }
  double GetDurationS() {
    return std::chrono::duration<double>(GetDuration()).count();
  }
  int64_t GetDurationMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(GetDuration())
        .count();
  }
  int64_t GetDurationUs() {
    return std::chrono::duration_cast<std::chrono::microseconds>(GetDuration())
        .count();
  }

  long long GetTimeout() { return timeout; }
};

}  // namespace dry
#endif