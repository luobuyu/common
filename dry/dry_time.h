#ifndef DRY_TIME_H
#define DRY_TIME_H
#include <string>
#include <ctime>
#include <chrono>
namespace dry
{
using clock = std::chrono::system_clock;

static std::string getTime(std::string format)
{
    auto now = clock::now();
    // 获取当前时间
    std::time_t t = clock::to_time_t(now);

    // 将时间转换为本地时间
    std::tm *local_time = std::localtime(&t);

    // 创建缓冲区来存储格式化日期
    char date_buffer[32];

    // 格式化输出日期（格式：YYYY-MM-DD）
    std::strftime(date_buffer, sizeof(date_buffer), format.c_str(), local_time);
    return std::string(date_buffer);
}

static std::string getTime(const clock::time_point& t, const std::string& format)
{
    // 将时间转换为本地时间
    std::time_t t_time_t= clock::to_time_t(t);
    std::tm *local_time = std::localtime(&t_time_t);

    // 创建缓冲区来存储格式化日期
    char date_buffer[32];

    // 格式化输出日期（格式：YYYY-MM-DD）
    std::strftime(date_buffer, sizeof(date_buffer), format.c_str(), local_time);
    return std::string(date_buffer);
}

class Timer
{
private:
    using TimePoint = clock::time_point;
    TimePoint startTime, endTime;
    long long timeout;

public:
    Timer() : startTime(clock::now()){}
    Timer(long long secTimeout)
    {
        timeout = secTimeout;
        setTimer(secTimeout);
    }
    void setTimer(long long secTimeout)
    {
        startTime = clock::now();
        endTime = startTime + std::chrono::seconds(secTimeout);
    }
    bool isTimeout()
    {
        return endTime < clock::now();
    }
    std::chrono::duration<double> getDuration()
    {
        return clock::now() - startTime;
    }
    double getDurationS()
    {
        return std::chrono::duration<double>(getDuration()).count();
    }
    double getDurationMs()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(getDuration()).count();
    }
    double getDurationUs()
    {
        return std::chrono::duration_cast<std::chrono::microseconds>(getDuration()).count();
    }

    long long getTimeout()
    {
        return timeout;
    }
};

} // namespace dry
#endif