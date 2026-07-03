#include "config.h"

#include <fstream>
#include <iostream>

#include "../dry_core/dry_string.h"
namespace dry {
namespace config {

bool Config::InitConfig(const std::string &path) {
  std::ifstream ifs(path, std::ios::in);
  if (!ifs.is_open()) {
    std::cerr << "can't open config file " << path << std::endl;
    return false;
  }
  // parser config
  std::string line, cur_section;
  while (std::getline(ifs, line)) {
    line = std::string(dry::Trim(line));

    // 忽略空行或注释行
    if (line.empty() || line.front() == '#' || line.front() == ';') continue;

    // 检查是否是section
    if (line.front() == '[' && line.back() == ']') {
      cur_section = line.substr(1, line.size() - 2);
      cur_section = std::string(dry::Trim(cur_section));
      m_sections[cur_section] = Section();
    }
    // 解析键值对
    else {
      size_t index = line.find_first_of('=');
      if (index == line.npos) {
        std::cerr << "can't parser config line: " << line << std::endl;
        return false;
      }
      std::string key(dry::Trim(line.substr(0, index)));
      std::string val(dry::Trim(line.substr(index + 1)));
      if (key.empty() || val.empty()) {
        std::cerr << "can't parser config line: " << line << std::endl;
        return false;
      }
      m_sections[cur_section][key] = val;
    }
  }
  ifs.close();
  return true;
}

bool Config::Exist(const std::string &section, const std::string &key) {
  auto section_iter = m_sections.find(section);
  if (section_iter == m_sections.end()) return false;
  if (section_iter->second.find(key) == section_iter->second.end()) {
    return false;
  }
  return true;
}
bool Config::Exist(const std::string &section) {
  return m_sections.find(section) != m_sections.end();
}

bool Config::GetValue(const std::string &section, const std::string &key,
                      std::string &val) {
  if (!Exist(section, key)) return false;
  val = m_sections[section][key];
  return true;
}
bool Config::GetValue(const std::string &section, const std::string &key,
                      int &val) {
  if (!Exist(section, key)) return false;
  val = std::stoi(m_sections[section][key]);
  return true;
}
bool Config::GetValue(const std::string &section, const std::string &key,
                      double &val) {
  if (!Exist(section, key)) return false;
  val = std::stod(m_sections[section][key]);
  return true;
}

void Config::GetValue(const std::string &section, const std::string &key,
                      std::string &val, const std::string &default_value) {
  if (!Exist(section, key)) {
    val = default_value;
    return;
  }
  val = m_sections[section][key];
}

void Config::GetValue(const std::string &section, const std::string &key,
                      int &val, const int &default_value) {
  if (!Exist(section, key)) {
    val = default_value;
    return;
  }
  val = std::stoi(m_sections[section][key]);
}
void Config::GetValue(const std::string &section, const std::string &key,
                      double &val, const double &default_value) {
  if (!Exist(section, key)) {
    val = default_value;
    return;
  }
  val = std::stod(m_sections[section][key]);
}

std::string Config::GetString(const std::string &section,
                              const std::string &key,
                              const std::string &default_value) {
  std::string value;
  GetValue(section, key, value, default_value);
  return value;
}
int Config::GetInt(const std::string &section, const std::string &key,
                   const int &default_value) {
  int value;
  GetValue(section, key, value, default_value);
  return value;
}
double Config::GetDouble(const std::string &section, const std::string &key,
                         const double &default_value) {
  double value;
  GetValue(section, key, value, default_value);
  return value;
}

std::unordered_map<std::string, Config::Section> Config::GetSections() {
  return m_sections;
}
std::vector<std::string> Config::GetSectionNames() {
  std::vector<std::string> sections;
  for (auto &[section, kvs] : m_sections) sections.emplace_back(section);
  return sections;
}

Config::Section Config::GetSectionKVs(const std::string &section) {
  if (m_sections.find(section) == m_sections.end()) {
    return {};
  } else {
    return m_sections[section];
  }
}
void Config::SetValue(const std::string &section, const std::string &key,
                      const std::string &val) {
  m_sections[section][key] = val;
}

};  // namespace config
}  // namespace dry