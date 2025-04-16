#include "config.h"

#include <fstream>
#include <iostream>

#include "../dry/dry_string.h"
namespace config {

bool Config::initConfig(const std::string &path) {
  std::ifstream ifs(path, std::ios::in);
  if (!ifs.is_open()) {
    std::cerr << "can't open config file " << path << std::endl;
    return false;
  }
  // parser config
  std::string line, cur_section;
  while (std::getline(ifs, line)) {
    line = dry::stringTrim(line);

    // 忽略空行或注释行
    if (line.empty() || line.front() == '#' || line.front() == ';') continue;

    // 检查是否是section
    if (line.front() == '[' && line.back() == ']') {
      cur_section = line.substr(1, line.size() - 2);
      cur_section = dry::stringTrim(cur_section);
      m_sections[cur_section] = Section();
    }
    // 解析键值对
    else {
      size_t index = line.find_first_of('=');
      if (index == line.npos) {
        std::cerr << "can't parser config line: " << line << std::endl;
        return false;
      }
      std::string key = dry::stringTrim(line.substr(0, index));
      std::string val = dry::stringTrim(line.substr(index + 1));
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

bool Config::exist(const std::string &section, const std::string &key) {
  auto section_iter = m_sections.find(section);
  if (section_iter == m_sections.end()) return false;
  if (section_iter->second.find(key) == section_iter->second.end()) {
    return false;
  }
  return true;
}
bool Config::exist(const std::string &section) {
  return m_sections.find(section) != m_sections.end();
}

bool Config::getValue(const std::string &section, const std::string &key,
                      std::string &val) {
  if (!exist(section, key)) return false;
  val = m_sections[section][key];
  return true;
}
bool Config::getValue(const std::string &section, const std::string &key,
                      int &val) {
  if (!exist(section, key)) return false;
  val = std::stoi(m_sections[section][key]);
  return true;
}
bool Config::getValue(const std::string &section, const std::string &key,
                      double &val) {
  if (!exist(section, key)) return false;
  val = std::stod(m_sections[section][key]);
  return true;
}

void Config::getValue(const std::string &section, const std::string &key,
                      std::string &val, const std::string &default_value) {
  if (!exist(section, key)) {
    val = default_value;
    return;
  }
  val = m_sections[section][key];
}

void Config::getValue(const std::string &section, const std::string &key,
                      int &val, const int &default_value) {
  if (!exist(section, key)) {
    val = default_value;
    return;
  }
  val = std::stoi(m_sections[section][key]);
}
void Config::getValue(const std::string &section, const std::string &key,
                      double &val, const double &default_value) {
  if (!exist(section, key)) {
    val = default_value;
    return;
  }
  val = std::stod(m_sections[section][key]);
}

std::string Config::getString(const std::string &section,
                              const std::string &key,
                              const std::string &default_value) {
  std::string value;
  getValue(section, key, value, default_value);
  return value;
}
int Config::getInt(const std::string &section, const std::string &key,
                   const int &default_value) {
  int value;
  getValue(section, key, value, default_value);
  return value;
}
double Config::getDouble(const std::string &section, const std::string &key,
                         const double &default_value) {
  double value;
  getValue(section, key, value, default_value);
  return value;
}

std::unordered_map<std::string, Config::Section> Config::getSections() {
  return m_sections;
}
std::vector<std::string> Config::getSectionNames() {
  std::vector<std::string> sections;
  for (auto &[section, kvs] : m_sections) sections.emplace_back(section);
  return sections;
}

Config::Section Config::getSectionKVs(const std::string &section) {
  if (m_sections.find(section) == m_sections.end()) {
    return {};
  } else {
    return m_sections[section];
  }
}
void Config::setValue(const std::string &section, const std::string &key,
                      const std::string &val) {
  m_sections[section][key] = val;
}

};  // namespace config