#ifndef DRY_STRING_H
#define DRY_STRING_H
#include <charconv>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>
namespace dry {

// 通用类型转换：将字符串转换为指定类型（失败抛异常）
// 数值类型走 std::from_chars（零分配、零 locale 依赖，性能最优）
// 非数值类型回退到 istringstream
template <typename T>
T strTo(std::string_view str) {
  if constexpr (std::is_arithmetic_v<T> && !std::is_same_v<T, bool>) {
    T value{};
    auto [ptr, ec] =
        std::from_chars(str.data(), str.data() + str.size(), value);
    if (ec != std::errc{}) {
      throw std::runtime_error("Failed to convert string to target type: " +
                               std::string(str));
    }
    return value;
  } else {
    std::istringstream iss{std::string(str)};
    T value;
    if (!(iss >> value)) {
      throw std::runtime_error("Failed to convert string to target type: " +
                               std::string(str));
    }
    return value;
  }
}

// std::string 特化 - 直接返回
template <>
inline std::string strTo<std::string>(std::string_view str) {
  return std::string(str);
}

// bool 特化 - 支持 "true", "false", "1", "0"
template <>
inline bool strTo<bool>(std::string_view str) {
  if (str == "true" || str == "1") return true;
  if (str == "false" || str == "0") return false;
  throw std::runtime_error("Invalid boolean value: " + std::string(str));
}

// 不抛异常版本：转换失败/字符串为空时返回 default_value
// 推荐在解析外部输入（HTTP header、query 参数等）时使用
// base 参数仅对整数类型生效（默认十进制；HTTP chunked size 等场景可传 16）
template <typename T>
T strToOr(std::string_view str, T default_value = T{}, int base = 10) noexcept {
  if (str.empty()) return default_value;
  if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
    T value{};
    auto [ptr, ec] =
        std::from_chars(str.data(), str.data() + str.size(), value, base);
    return ec == std::errc{} ? value : default_value;
  } else if constexpr (std::is_floating_point_v<T>) {
    T value{};
    auto [ptr, ec] =
        std::from_chars(str.data(), str.data() + str.size(), value);
    return ec == std::errc{} ? value : default_value;
  } else if constexpr (std::is_same_v<T, bool>) {
    if (str == "true" || str == "1") return true;
    if (str == "false" || str == "0") return false;
    return default_value;
  } else if constexpr (std::is_same_v<T, std::string>) {
    return std::string(str);
  } else {
    std::istringstream iss{std::string(str)};
    T value;
    if (!(iss >> value)) return default_value;
    return value;
  }
}

// split
inline std::vector<std::string> stringSplit(const std::string &s,
                                            const std::string &delim) {
  std::vector<std::string> ret;
  size_t start = 0, end = 0;
  size_t len = s.length();
  // 逐个找到子串并分割
  while ((end = s.find(delim, start)) != std::string::npos) {
    std::string substr = s.substr(start, end - start);
    if (!substr.empty()) ret.emplace_back(substr);  // 提取子串
    start = end + delim.length();                   // 跳过分隔符
  }
  // 处理最后一个部分
  std::string substr = s.substr(start);
  if (!substr.empty()) ret.emplace_back(substr);
  return ret;
}
// split
inline std::vector<std::string> stringSplit(const std::string &str,
                                            char delim = ' ') {
  return stringSplit(str, std::string(1, delim));
}

// split (string_view 版本，零拷贝；返回的 view 生命周期由调用方保证 s 仍存活)
// 注意：与 stringSplit 行为一致——跳过空段
inline std::vector<std::string_view> stringSplitView(std::string_view s,
                                                     std::string_view delim) {
  std::vector<std::string_view> ret;
  if (delim.empty()) {
    if (!s.empty()) ret.emplace_back(s);
    return ret;
  }
  size_t start = 0, end = 0;
  while ((end = s.find(delim, start)) != std::string_view::npos) {
    if (end > start) ret.emplace_back(s.substr(start, end - start));
    start = end + delim.size();
  }
  if (start < s.size()) ret.emplace_back(s.substr(start));
  return ret;
}

inline std::vector<std::string_view> stringSplitView(std::string_view s,
                                                     char delim = ' ') {
  std::vector<std::string_view> ret;
  size_t start = 0, end = 0;
  while ((end = s.find(delim, start)) != std::string_view::npos) {
    if (end > start) ret.emplace_back(s.substr(start, end - start));
    start = end + 1;
  }
  if (start < s.size()) ret.emplace_back(s.substr(start));
  return ret;
}

// join - 将字符串数组用分隔符拼接为一个字符串（stringSplit 的逆操作）
inline std::string stringJoin(const std::vector<std::string> &parts,
                              const std::string &delim) {
  std::string result;
  for (size_t i = 0; i < parts.size(); i++) {
    if (i > 0) result += delim;
    result += parts[i];
  }
  return result;
}

inline std::string stringTrim(const std::string &str) {
  size_t first = str.find_first_not_of(" \t");
  if (first == std::string::npos) return "";
  size_t last = str.find_last_not_of(" \t");
  return str.substr(first, last - first + 1);
}

// 字符串转小写
inline std::string toLower(std::string_view str) {
  std::string result(str);
  for (auto &c : result) {
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  }
  return result;
}

// 字符串转大写
inline std::string toUpper(std::string_view str) {
  std::string result(str);
  for (auto &c : result) {
    c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
  }
  return result;
}

// 大小写不敏感比较
inline bool equalsIgnoreCase(std::string_view a, std::string_view b) {
  if (a.size() != b.size()) return false;
  for (size_t i = 0; i < a.size(); ++i) {
    if (std::tolower(static_cast<unsigned char>(a[i])) !=
        std::tolower(static_cast<unsigned char>(b[i]))) {
      return false;
    }
  }
  return true;
}

// 通用数值转字符串（基于 std::to_chars，零 locale 依赖，比 std::to_string
// 更快） 返回 std::string（短数值在 SSO 范围内零堆分配）
template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
inline std::string toString(T value) {
  char buf[32];
  auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), value);
  if (ec != std::errc()) {
    return "";
  }
  return std::string(buf, ptr - buf);
}

template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
inline std::string toString(T value) {
  char buf[64];
  auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), value);
  if (ec != std::errc()) {
    return "";
  }
  return std::string(buf, ptr - buf);
}

};  // namespace dry

#endif