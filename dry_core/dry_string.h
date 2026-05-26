#ifndef DRY_STRING_H
#define DRY_STRING_H
#include <charconv>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>
namespace dry {

// 严格类型转换：将字符串转换为指定类型，返回 std::optional
// 要求整个字符串都被消费（不允许部分匹配，如 "42abc" → nullopt）
// 数值类型走 std::from_chars（零分配、零 locale 依赖，性能最优）
// 非数值类型回退到 istringstream
// base 参数仅对整数类型生效（默认十进制；HTTP chunked size 等场景可传 16）
template <typename T>
std::optional<T> strTo(std::string_view str, int base = 10) noexcept {
  if (str.empty()) return std::nullopt;
  if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
    T value{};
    auto [ptr, ec] =
        std::from_chars(str.data(), str.data() + str.size(), value, base);
    // 必须无错误 且 消费了全部字符
    if (ec != std::errc{} || ptr != str.data() + str.size()) {
      return std::nullopt;
    }
    return value;
  } else if constexpr (std::is_floating_point_v<T>) {
    T value{};
    auto [ptr, ec] =
        std::from_chars(str.data(), str.data() + str.size(), value);
    if (ec != std::errc{} || ptr != str.data() + str.size()) {
      return std::nullopt;
    }
    return value;
  } else if constexpr (std::is_same_v<T, bool>) {
    if (str == "true" || str == "1") return true;
    if (str == "false" || str == "0") return false;
    return std::nullopt;
  } else if constexpr (std::is_same_v<T, std::string>) {
    return std::string(str);
  } else {
    std::istringstream iss{std::string(str)};
    T value;
    if (!(iss >> value)) return std::nullopt;
    // 检查是否还有剩余字符（跳过尾部空白）
    char c;
    if (iss >> c) return std::nullopt;  // 还有未消费的非空白字符
    return value;
  }
}

// 默认值版本：转换失败/字符串为空时返回 default_value
// 内部调用 strTo，同样是严格模式（要求消费全部字符）
// 推荐在解析外部输入且只需要兜底值的场景使用（如 query 参数、Range 头等）
// base 参数仅对整数类型生效（默认十进制；HTTP chunked size 等场景可传 16）
template <typename T>
T strToOr(std::string_view str, T default_value = T{}, int base = 10) noexcept {
  return strTo<T>(str, base).value_or(default_value);
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

// trimView：去除前后 SP/HTAB，返回 string_view（零拷贝）
// 注意：返回的 string_view 生命周期依赖于传入的 sv 所指向的内存，
// 调用方需确保底层数据在使用期间存活（勿对临时 string 的返回值持有 view）
inline std::string_view trimView(std::string_view sv) {
  while (!sv.empty() && (sv.front() == ' ' || sv.front() == '\t')) {
    sv.remove_prefix(1);
  }
  while (!sv.empty() && (sv.back() == ' ' || sv.back() == '\t')) {
    sv.remove_suffix(1);
  }
  return sv;
}

// trim：去除前后 SP/HTAB，返回 std::string（安全拷贝，可对临时对象使用）
inline std::string trim(std::string_view sv) {
  return std::string(trimView(sv));
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