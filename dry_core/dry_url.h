#pragma once
#include <cctype>
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
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
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
        decoded += static_cast<char>((h << 4) | l);
        i += 2;  // 跳过两个十六进制字符
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

}  // namespace dry
