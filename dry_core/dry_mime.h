
#pragma once
#include <string>
#include <unordered_map>

#include "dry_core/dry_string.h"

namespace dry {

/// 根据文件路径或扩展名获取对应的 MIME 类型
/// @param path 文件路径或扩展名（如 "index.html" 或 ".html"）
/// @return 对应的 MIME 类型字符串，未知类型返回 "application/octet-stream"
inline std::string GetMimeType(const std::string& path) {
  static const std::unordered_map<std::string, std::string> mime_types = {
      // 文本类型
      {".html", "text/html"},
      {".htm", "text/html"},
      {".css", "text/css"},
      {".csv", "text/csv"},
      {".txt", "text/plain"},
      {".xml", "application/xml"},
      {".xhtml", "application/xhtml+xml"},

      // JavaScript / JSON / WebAssembly
      {".js", "application/javascript"},
      {".mjs", "application/javascript"},
      {".json", "application/json"},
      {".wasm", "application/wasm"},

      // 图片类型
      {".png", "image/png"},
      {".jpg", "image/jpeg"},
      {".jpeg", "image/jpeg"},
      {".gif", "image/gif"},
      {".bmp", "image/bmp"},
      {".svg", "image/svg+xml"},
      {".ico", "image/x-icon"},
      {".webp", "image/webp"},
      {".avif", "image/avif"},

      // 音频类型
      {".mp3", "audio/mpeg"},
      {".wav", "audio/wav"},
      {".ogg", "audio/ogg"},
      {".flac", "audio/flac"},
      {".aac", "audio/aac"},

      // 视频类型
      {".mp4", "video/mp4"},
      {".webm", "video/webm"},
      {".avi", "video/x-msvideo"},
      {".mkv", "video/x-matroska"},
      {".mov", "video/quicktime"},

      // 字体类型
      {".woff", "font/woff"},
      {".woff2", "font/woff2"},
      {".ttf", "font/ttf"},
      {".otf", "font/otf"},
      {".eot", "application/vnd.ms-fontobject"},

      // 压缩 / 归档
      {".zip", "application/zip"},
      {".gz", "application/gzip"},
      {".tar", "application/x-tar"},
      {".7z", "application/x-7z-compressed"},
      {".rar", "application/vnd.rar"},

      // 文档类型
      {".pdf", "application/pdf"},
      {".doc", "application/msword"},
      {".docx",
       "application/vnd.openxmlformats-officedocument.wordprocessingml."
       "document"},
      {".xls", "application/vnd.ms-excel"},
      {".xlsx",
       "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
      {".ppt", "application/vnd.ms-powerpoint"},
      {".pptx",
       "application/vnd.openxmlformats-officedocument.presentationml."
       "presentation"},

      // 其他
      {".bin", "application/octet-stream"},
      {".sh", "application/x-sh"},
      {".yaml", "application/x-yaml"},
      {".yml", "application/x-yaml"},
      {".toml", "application/toml"},
      {".md", "text/markdown"},
      {".map", "application/json"},
  };

  auto dot_pos = path.rfind('.');
  if (dot_pos == std::string::npos) {
    return "application/octet-stream";
  }

  std::string ext = dry::ToLower(path.substr(dot_pos));

  auto it = mime_types.find(ext);
  if (it != mime_types.end()) {
    return it->second;
  }
  return "application/octet-stream";
}

/// 判断 MIME 类型是否为文本类型（可用于决定是否添加 charset=utf-8）
/// @param mime_type MIME 类型字符串
/// @return 是否为文本类型
inline bool IsTextMime(const std::string& mime_type) {
  // text/* 开头的都是文本类型
  if (mime_type.compare(0, 5, "text/") == 0) {
    return true;
  }
  // 部分 application/* 也是文本类型
  return mime_type == "application/json" ||
         mime_type == "application/javascript" ||
         mime_type == "application/xml" ||
         mime_type == "application/xhtml+xml" ||
         mime_type == "application/x-yaml" || mime_type == "application/toml";
}

/// 根据 MIME 类型反向查找扩展名
/// @param mime_type MIME 类型字符串
/// @return 对应的扩展名（含点号），未知类型返回 ".bin"
inline std::string GetExtension(const std::string& mime_type) {
  static const std::unordered_map<std::string, std::string> ext_map = {
      {"text/html", ".html"},        {"text/css", ".css"},
      {"text/plain", ".txt"},        {"text/csv", ".csv"},
      {"text/markdown", ".md"},      {"application/javascript", ".js"},
      {"application/json", ".json"}, {"application/xml", ".xml"},
      {"application/pdf", ".pdf"},   {"application/zip", ".zip"},
      {"application/gzip", ".gz"},   {"application/wasm", ".wasm"},
      {"image/png", ".png"},         {"image/jpeg", ".jpg"},
      {"image/gif", ".gif"},         {"image/svg+xml", ".svg"},
      {"image/webp", ".webp"},       {"image/x-icon", ".ico"},
      {"audio/mpeg", ".mp3"},        {"audio/wav", ".wav"},
      {"audio/ogg", ".ogg"},         {"video/mp4", ".mp4"},
      {"video/webm", ".webm"},       {"font/woff", ".woff"},
      {"font/woff2", ".woff2"},      {"font/ttf", ".ttf"},
  };

  auto it = ext_map.find(mime_type);
  if (it != ext_map.end()) {
    return it->second;
  }
  return ".bin";
}

}  // namespace dry
