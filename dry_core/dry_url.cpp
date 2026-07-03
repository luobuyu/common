#include "dry_core/dry_url.h"

#include <cctype>

#include "dry_core/dry_string.h"

namespace dry {

/// 安全的端口解析（不抛异常，范围校验 0~65535）
uint16_t SafeParsePort(std::string_view port_str) {
  return StrTo<uint16_t>(port_str).value_or(0);
}
/// 解析 authority 部分：[userinfo@]host[:port][/path][?query]
/// @note 内部辅助函数，由 ParseUrl 调用
static void ParseAuthority(std::string_view authority, UrlComponents& result) {
  if (authority.empty()) {
    result.path = "/";
    return;
  }

  // ====== Step 1: 分离 authority 和 path?query ======
  // authority 终止于第一个 '/' 或 '?'（漏洞 #11）
  size_t authority_end = std::string_view::npos;
  for (size_t i = 0; i < authority.size(); ++i) {
    if (authority[i] == '/' || authority[i] == '?') {
      authority_end = i;
      break;
    }
  }

  std::string host_part;
  std::string path_query;
  if (authority_end != std::string_view::npos) {
    host_part = std::string(authority.substr(0, authority_end));
    path_query = std::string(authority.substr(authority_end));
  } else {
    host_part = std::string(authority);
    path_query = "/";
  }

  // 分离 path 和 query（漏洞 #10）
  if (!path_query.empty() && path_query[0] == '?') {
    // 无 path，只有 query
    result.path = "/";
    result.query = path_query;
  } else {
    auto q_pos = path_query.find('?');
    if (q_pos != std::string::npos) {
      result.path = path_query.substr(0, q_pos);
      result.query = path_query.substr(q_pos);
    } else {
      result.path = path_query.empty() ? "/" : path_query;
    }
  }

  // ====== Step 2: 剥离 userinfo（漏洞 #4）======
  // RFC 3986: authority = [userinfo "@"] host [":" port]
  // 安全考虑：直接丢弃 userinfo，只取 @ 后面的部分
  auto at_pos = host_part.rfind('@');
  if (at_pos != std::string::npos) {
    host_part = host_part.substr(at_pos + 1);
  }

  // ====== Step 3: 处理反斜杠（漏洞 #5）======
  // 某些实现把 '\' 当 '/' 处理，这可能导致解析混乱
  // 安全做法：如果 host_part 中包含 '\'，视为无效 URL
  if (host_part.find('\\') != std::string::npos) {
    result.valid = false;
    return;
  }

  // ====== Step 4: 解析 host:port ======
  if (host_part.empty()) {
    // 空 host（漏洞 #13：如 http:///path）
    result.path = result.path.empty() ? "/" : result.path;
    return;
  }

  if (host_part[0] == '[') {
    // IPv6 地址：[::1]:8080（漏洞 #1）
    auto bracket_end = host_part.find(']');
    if (bracket_end == std::string::npos) {
      // 格式错误，不跟随
      result.valid = false;
      return;
    }
    result.host = host_part.substr(1, bracket_end - 1);  // 去掉 []
    if (bracket_end + 1 < host_part.size() && host_part[bracket_end + 1] == ':') {
      result.port = SafeParsePort(std::string_view(host_part).substr(bracket_end + 2));
    }
  } else {
    // IPv4 / hostname
    auto colon = host_part.rfind(':');
    if (colon != std::string::npos) {
      // 验证 colon 后面全是数字（防止 IPv6 裸地址误判）
      bool all_digits = true;
      for (size_t i = colon + 1; i < host_part.size(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(host_part[i]))) {
          all_digits = false;
          break;
        }
      }
      if (all_digits && colon + 1 < host_part.size()) {
        result.host = host_part.substr(0, colon);
        result.port = SafeParsePort(std::string_view(host_part).substr(colon + 1));
      } else {
        // colon 后面不是纯数字，整个当 host（可能是 IPv6 裸地址）
        result.host = host_part;
      }
    } else {
      result.host = host_part;
    }
  }

  // host 转小写（漏洞 #6：RFC 3986 host is case-insensitive）
  for (auto& c : result.host) {
    c = std::tolower(static_cast<unsigned char>(c));
  }
}

/// RFC 3986 §5.2.4 相对路径解析（dot-segment 移除）
std::string ResolveRelativePath(std::string_view relative, std::string_view base_path) {
  // 先分离 query string（避免 query 中的 ".." 等干扰路径解析）（漏洞 B）
  std::string path_part(relative);
  std::string query_part;
  auto q_pos = relative.find('?');
  if (q_pos != std::string_view::npos) {
    path_part = std::string(relative.substr(0, q_pos));
    query_part = std::string(relative.substr(q_pos));  // 含 '?'
  }

  // 取 base_path 的目录部分
  // 例如 base_path = "/api/v1/users"，目录 = "/api/v1/"
  // 注意：base_path 可能包含 query string（如 "/search?url=/foo/bar"），
  // 必须先剥离 query 再做 rfind('/')，否则 query 中的 '/' 会干扰目录计算（漏洞
  // F）
  std::string base = "/";
  if (!base_path.empty()) {
    auto query_start = base_path.find('?');
    std::string_view pure_path =
        (query_start != std::string_view::npos) ? base_path.substr(0, query_start) : base_path;
    auto last_slash = pure_path.rfind('/');
    if (last_slash != std::string_view::npos) {
      base = std::string(pure_path.substr(0, last_slash + 1));
    }
  }

  std::string merged = base + path_part;

  // RFC 3986 §5.2.4: Remove Dot Segments
  std::string output;
  output.reserve(merged.size());
  size_t pos = 0;
  while (pos < merged.size()) {
    // A: "../" 或 "./" 开头 → 移除
    if (merged.compare(pos, 3, "../") == 0) {
      pos += 3;
      continue;
    }
    if (merged.compare(pos, 2, "./") == 0) {
      pos += 2;
      continue;
    }
    // B: "/./" 或 "/." 结尾 → 替换为 "/"
    if (merged.compare(pos, 3, "/./") == 0) {
      pos += 2;
      continue;
    }
    if (pos + 2 == merged.size() && merged.compare(pos, 2, "/.") == 0) {
      merged[pos + 1] = '/';
      merged.resize(pos + 2);
      continue;
    }
    // C: "/../" 或 "/.." 结尾 → 替换为 "/"，回退上一段
    if (merged.compare(pos, 4, "/../") == 0) {
      pos += 3;
      auto last = output.rfind('/');
      if (last != std::string::npos) {
        output.resize(last);
      }
      continue;
    }
    if (pos + 3 == merged.size() && merged.compare(pos, 3, "/..") == 0) {
      auto last = output.rfind('/');
      if (last != std::string::npos) {
        output.resize(last);
      }
      output += '/';
      break;
    }
    // D: 单独的 "." 或 ".." → 移除
    if ((merged.size() - pos == 1 && merged[pos] == '.') ||
        (merged.size() - pos == 2 && merged[pos] == '.' && merged[pos + 1] == '.')) {
      break;
    }
    // E: 移动第一个路径段到 output
    size_t seg_start = pos;
    if (merged[pos] == '/') {
      pos++;
    }
    while (pos < merged.size() && merged[pos] != '/') {
      pos++;
    }
    output += merged.substr(seg_start, pos - seg_start);
  }

  std::string result_path = output.empty() ? "/" : output;
  return result_path + query_part;  // 重新拼接 query string
}

/// 解析 URL（支持绝对 URL、相对路径、protocol-relative）
UrlComponents ParseUrl(std::string_view location, std::string_view base_path) {
  UrlComponents result;
  if (location.empty()) {
    result.valid = false;
    return result;
  }

  // ====== Step 0: trim 前后空白（漏洞 #9）======
  size_t start = 0, end = location.size();
  while (start < end && (location[start] == ' ' || location[start] == '\t')) {
    start++;
  }
  while (end > start && (location[end - 1] == ' ' || location[end - 1] == '\t')) {
    end--;
  }
  if (start >= end) {
    result.valid = false;
    return result;
  }
  std::string loc(location.substr(start, end - start));

  // ====== Step 1: 剥离 fragment（漏洞 #8）======
  // fragment 不应发送给服务端
  auto frag_pos = loc.find('#');
  if (frag_pos != std::string::npos) {
    loc = loc.substr(0, frag_pos);
  }
  if (loc.empty()) {
    result.valid = false;
    return result;
  }

  // ====== Step 2: 判断 URL 类型 ======

  // 2a. 相对路径（以 "/" 开头，但不是 "//"）
  if (loc[0] == '/' && (loc.size() < 2 || loc[1] != '/')) {
    // 分离 path 和 query（漏洞 #10）
    auto q_pos2 = loc.find('?');
    if (q_pos2 != std::string::npos) {
      result.path = loc.substr(0, q_pos2);
      result.query = loc.substr(q_pos2);  // 含 '?'
    } else {
      result.path = loc;
    }
    return result;
  }

  // 2b. protocol-relative（以 "//" 开头）
  if (loc.size() >= 2 && loc[0] == '/' && loc[1] == '/') {
    ParseAuthority(std::string_view(loc).substr(2), result);
    return result;
  }

  // 2c. 判断是否是绝对 URL
  //     关键修复（漏洞 #3）：只在第一个 '?', '/', '#' 之前查找 ":"
  //     RFC 3986: scheme = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
  //     scheme 后面紧跟 "://"
  size_t colon_pos = loc.find(':');
  if (colon_pos == std::string::npos || colon_pos == 0) {
    // 无 scheme → 相对路径（漏洞 #7 修复：正确解析相对路径）
    result.path = ResolveRelativePath(loc, base_path);
    return result;
  }

  // 检查 colon 之前是否全是合法 scheme 字符
  bool valid_scheme = true;
  for (size_t i = 0; i < colon_pos; ++i) {
    char c = loc[i];
    if (!std::isalnum(static_cast<unsigned char>(c)) && c != '+' && c != '-' && c != '.') {
      valid_scheme = false;
      break;
    }
    // 第一个字符必须是 ALPHA
    if (i == 0 && !std::isalpha(static_cast<unsigned char>(c))) {
      valid_scheme = false;
      break;
    }
  }

  if (!valid_scheme || loc.size() < colon_pos + 3 || loc[colon_pos + 1] != '/' ||
      loc[colon_pos + 2] != '/') {
    // 不是合法的 scheme://... → 当作相对路径
    result.path = ResolveRelativePath(loc, base_path);
    return result;
  }

  // 2d. 绝对 URL
  result.scheme = loc.substr(0, colon_pos);
  // scheme 转小写（RFC 3986: scheme is case-insensitive）
  for (auto& c : result.scheme) {
    c = std::tolower(static_cast<unsigned char>(c));
  }

  // 漏洞 #14：只允许 http/https
  if (result.scheme != "http" && result.scheme != "https") {
    // 非 HTTP scheme，标记为无效（不跟随重定向）
    result.valid = false;
    return result;
  }

  ParseAuthority(std::string_view(loc).substr(colon_pos + 3), result);
  return result;
}

/// 判断两个 URL 是否同源（scheme + host + EffectivePort 均相同）
/// @param origin 当前连接的 origin（scheme/host/port）
/// @param target 目标 URL（如 Location 头解析结果）
/// @return true 表示同源，可以跟随重定向
/// @note 纯函数，无状态
/// @note 如果 target 是相对路径（host 为空），视为同源
/// @note 如果 target.valid == false，视为不同源（不跟随）
bool IsSameOrigin(const UrlComponents& origin, const UrlComponents& target) {
  if (!target.valid) {
    return false;
  }
  if (target.IsRelative()) {
    return true;  // 相对路径一定同源
  }

  // host 比较（两者都已转小写）
  if (origin.host != target.host) {
    return false;
  }

  // port 比较（考虑默认端口推断）
  // 如果 target.scheme 为空（protocol-relative），继承 origin 的 scheme
  // 来推断端口
  uint16_t target_port = target.EffectivePort();
  if (target_port == 0 && target.scheme.empty()) {
    // protocol-relative URL，用 origin 的端口作为默认值
    target_port = origin.EffectivePort();
  }

  return origin.EffectivePort() == target_port;
}

}  // namespace dry