#ifndef DRY_STRING_H
#define DRY_STRING_H
#include <sstream>
#include <string>
#include <vector>
namespace dry {

// 通用类型转换：将字符串转换为指定类型
// 使用 std::istringstream 进行转换
template <typename T>
T strTo(const std::string& str) {
  std::istringstream iss(str);
  T value;
  if (!(iss >> value)) {
    throw std::runtime_error("Failed to convert string to target type: " + str);
  }
  return value;
}

// std::string 特化 - 直接返回
template <>
inline std::string strTo<std::string>(const std::string& str) {
  return str;
}

// bool 特化 - 支持 "true", "false", "1", "0"
template <>
inline bool strTo<bool>(const std::string& str) {
  if (str == "true" || str == "1") return true;
  if (str == "false" || str == "0") return false;
  throw std::runtime_error("Invalid boolean value: " + str);
}

// split
static std::vector<std::string> stringSplit(const std::string &s,
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
static std::vector<std::string> stringSplit(const std::string &str,
                                            char delim = ' ') {
  return stringSplit(str, std::string(1, delim));
}

static std::string stringTrim(const std::string &str) {
  size_t first = str.find_first_not_of(" \t");
  if (first == std::string::npos) return "";
  size_t last = str.find_last_not_of(" \t");
  return str.substr(first, last - first + 1);
}
};  // namespace dry

#endif