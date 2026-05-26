#pragma once
#include <string_view>

namespace dry {

/// HTTP 协议字符校验工具（查表法，O(1) 查询）
/// 参考 RFC 7230 / RFC 3986，与 dry-basic utils/dry_encoding.cpp
/// TOKEN_CHAR[256] 对齐

// clang-format off

/// Header name 合法字符表（RFC 7230 §3.2.6 token 字符集）
/// token = 1*tchar
/// tchar = "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" / "." /
///         "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA
/// 自然覆盖 NULL、控制字符、空格等所有非法字符
inline constexpr bool kHttpHeaderNameChar[256] = {
    // 0x00-0x0F: 控制字符，全部非法
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    // 0x10-0x1F: 控制字符，全部非法
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    // 0x20-0x2F: SP ! " # $ % & ' ( ) * + , - . /
    false, true,  false, true,  true,  true,  true,  true,  false, false, true,  true,  false, true,  true,  false,
    // 0x30-0x3F: 0-9 : ; < = > ?
    true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  false, false, false, false, false, false,
    // 0x40-0x4F: @ A-O
    false, true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,
    // 0x50-0x5F: P-Z [ \ ] ^ _
    true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  false, false, false, true,  true,
    // 0x60-0x6F: ` a-o
    true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,
    // 0x70-0x7F: p-z { | } ~ DEL
    true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  false, true,  false, true,  false,
    // 0x80-0xFF: 高位字节，全部非法
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
};

/// Header value 非法字符表
/// 拒绝 NULL(0x00)、控制字符(0x01-0x08, 0x0A-0x1F)、DEL(0x7F)
/// TAB(0x09)、可打印字符(0x20-0x7E)、obs-text(0x80-0xFF) 均合法
inline constexpr bool kHttpHeaderValueInvalid[256] = {
    // 0x00-0x0F: NULL 非法，0x01-0x08 非法，TAB(0x09) 合法，0x0A(LF) 非法，
    //            0x0B-0x0C 非法，0x0D(CR) 非法，0x0E-0x0F 非法
    true,  true,  true,  true,  true,  true,  true,  true,  true,  false, true,  true,  true,  true,  true,  true,
    // 0x10-0x1F: 控制字符，全部非法
    true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,
    // 0x20-0x7E: 可打印字符，全部合法
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    // 0x7F: DEL，非法
    true,
    // 0x80-0xFF: obs-text，合法（RFC 7230 允许）
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
};

// clang-format on

/// 校验 header name 是否为合法 HTTP token 字符串（RFC 7230 §3.2.6）
/// 空字符串视为非法
inline bool isValidHttpHeaderName(std::string_view sv) {
  if (sv.empty()) return false;
  for (char c : sv) {
    if (!kHttpHeaderNameChar[static_cast<unsigned char>(c)]) return false;
  }
  return true;
}

/// 校验 header value 是否包含非法字符（NULL / 裸 CR / 裸 LF / 控制字符）
inline bool isValidHttpHeaderValue(std::string_view sv) {
  for (char c : sv) {
    if (kHttpHeaderValueInvalid[static_cast<unsigned char>(c)]) return false;
  }
  return true;
}

/// 校验 URL 是否包含控制字符（0x00-0x1F）或 DEL（0x7F）
/// 规则简单，不需要查表
inline bool isValidHttpUrl(std::string_view sv) {
  for (char c : sv) {
    auto uc = static_cast<unsigned char>(c);
    if (uc < 0x20 || uc == 0x7F) return false;
  }
  return true;
}

/// 校验 Host header 值是否合法（防止 Host header 攻击 / SSRF）
/// 拒绝 '/' 和 '@'：
///   - '/' 防止路径注入（Host 被拼接到 URL 时）
///   - '@' 防止 userinfo 注入（如 evil@host 被误解析）
/// Go net/http httpguts.ValidHostHeader() 做了类似校验。
/// 注意：控制字符 / NULL / CR / LF 已由 isValidHttpHeaderValue 拦截，
///       此函数仅做 Host 特有的额外校验。
inline bool isValidHostValue(std::string_view sv) {
  for (char c : sv) {
    if (c == '/' || c == '@') return false;
  }
  return true;
}

}  // namespace dry
