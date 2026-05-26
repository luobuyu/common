#pragma once
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>

namespace dry {

/// URL 解码：将 %XX 编码转换为对应字符，+ 转换为空格
/// @param str 待解码的 URL 编码字符串（接受 std::string / const char* /
///        std::string_view，均可零拷贝传入）
/// @return 解码后的字符串
inline std::string urlDecode(std::string_view str) {
  // 单字符 hex 转 0~15，非法返回 -1。比 sscanf("%2x") 快 5~10 倍，
  // 且不依赖 c_str() 的 null 终止——string_view 不保证以 \0 结尾。
  auto fromHex = [](char c) -> int {
    if (c >= '0' && c <= '9') {
      return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
      return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
      return c - 'A' + 10;
    }
    return -1;
  };
  std::string decoded;
  decoded.reserve(str.size());
  for (size_t i = 0; i < str.size(); ++i) {
    if (str[i] == '+') {
      decoded += ' ';
    } else if (str[i] == '%' && i + 2 < str.size()) {
      int h = fromHex(str[i + 1]);
      int l = fromHex(str[i + 2]);
      if (h >= 0 && l >= 0) {
        char decoded_char = static_cast<char>((h << 4) | l);
        if (decoded_char == '\0') {
          // NULL 字节不解码，保留原样 %00，防止路径截断攻击
          decoded += str[i];
        } else {
          decoded += decoded_char;
          i += 2;  // 跳过两个十六进制字符
        }
      } else {
        // 非法的 % 编码，保留原样
        decoded += str[i];
      }
    } else {
      decoded += str[i];
    }
  }
  return decoded;
}

/// URL 编码：将特殊字符转换为 %XX 格式
/// @param str 待编码的原始字符串
/// @return 编码后的 URL 安全字符串
inline std::string urlEncode(const std::string& str) {
  std::string encoded;
  encoded.reserve(str.size() * 3);  // 最坏情况每个字符变3字节
  for (unsigned char c : str) {
    if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      // RFC 3986 未保留字符，直接保留
      encoded += static_cast<char>(c);
    } else if (c == ' ') {
      encoded += '+';
    } else {
      // 其他字符进行 %XX 编码
      char buf[4];
      std::snprintf(buf, sizeof(buf), "%%%02X", c);
      encoded += buf;
    }
  }
  return encoded;
}

/// URL 组成部分（通用结构，HttpClient/HttpSession/路由匹配等均可使用）
struct UrlComponents {
  std::string scheme;  // "http"/"https"，相对路径时为空（已转小写）
  std::string host;    // 主机名/IP（已转小写），相对路径时为空
  uint16_t port{0};  // 端口，0 表示未指定（需根据 scheme 推断默认值）
  std::string path;   // 路径（不含 query 和 fragment）
  std::string query;  // 查询字符串（含前导 '?'）
  bool valid{true};  // 解析是否成功（false 表示 URL 格式非法或不支持的 scheme）

  /// 返回完整的 path+query（用于发送请求和循环检测）
  std::string fullPath() const { return path + query; }

  /// 是否为相对路径（host 为空且解析成功）
  bool isRelative() const { return host.empty() && valid; }

  /// 是否为绝对 URL（host 非空且解析成功）
  bool isAbsolute() const { return !host.empty() && valid; }

  /// 获取有效端口（未指定时根据 scheme 推断默认值）
  uint16_t effectivePort() const {
    if (port != 0) {
      return port;
    }
    if (scheme == "https") {
      return 443;
    }
    if (scheme == "http") {
      return 80;
    }
    return 0;  // 无法推断
  }
};

/// 解析 URL（支持绝对 URL、相对路径、protocol-relative）
/// @param location  URL 字符串（如 Location 头的值）
/// @param base_path 当前请求的 path（用于解析无前导 / 的相对路径），可为空
/// @return 解析后的 URL 组成部分，解析失败时 valid=false
/// @note 纯函数，无状态，可在任何上下文中调用
UrlComponents parseUrl(std::string_view location,
                       std::string_view base_path = "");

/// 安全端口解析（不抛异常，范围校验 0~65535）
/// @return 解析成功返回端口号，失败返回 0
uint16_t safeParsePort(std::string_view port_str);

/// RFC 3986 §5.2.4 相对路径解析（dot-segment 移除）
/// @param relative  相对路径（可含 query，如 "../v2/users?page=1"）
/// @param base_path 当前请求的 path（用于确定目录基准）
/// @return 解析后的绝对路径（含 query）
std::string resolveRelativePath(std::string_view relative,
                                std::string_view base_path);

/// 判断两个 URL 是否同源（scheme + host + effectivePort 均相同）
/// @param origin 当前连接的 origin（scheme/host/port）
/// @param target 目标 URL（如 Location 头解析结果）
/// @return true 表示同源，可以跟随重定向
/// @note 纯函数，无状态
/// @note 如果 target 是相对路径（host 为空），视为同源
/// @note 如果 target.valid == false，视为不同源（不跟随）
bool isSameOrigin(const UrlComponents& origin, const UrlComponents& target);
}  // namespace dry
