#include "argument.h"

// 使用默认参数的构造函数
Argument::Argument(const std::vector<std::string>& names, 
                   const std::string& description,
                   bool required,
                   std::function<void()> callback)
  : m_names{names}, m_description{description}, m_required{required}, 
    m_parsed{false}, m_callback{callback} {
}

Argument::Argument(const ArgumentType& type,
                   const std::vector<std::string>& names,
                   const std::string& description, bool required,
                   std::function<void()> callback)
  : m_type{type}, m_names{names}, m_description{description}, m_required{required},
    m_parsed{false}, m_callback{callback} {}

void Argument::setDescription(const std::string& description) {
  m_description = description;
}

void Argument::setRequired(bool required) {
  m_required = required;
}

void Argument::setParsed(bool parsed) {
  m_parsed = parsed;
}

void Argument::setCallback(std::function<void()> callback) {
  m_callback = callback;
}

const std::vector<std::string>& Argument::getNames() const {
  return m_names;
}

const std::string& Argument::getDescription() const {
  return m_description;
}

ArgumentType Argument::getType() const {
  return m_type;
}

bool Argument::isRequired() const {
  return m_required;
}

bool Argument::isParsed() const {
  return m_parsed;
}

bool Argument::isOption(const std::string& argument) const { 
  if(argument.size() < 2 || argument[0] != '-') return false;
  // argument[0] == '-' && argument.size() >= 2
  
  // 长选项: --xxx
  if (argument[1] == '-') return true;
  
  // 尝试判断是否为负数
  try {
    size_t pos = 0;
    std::stod(argument, &pos);
    // 如果完全解析成功，说明是纯数字（如 -123, -0.5）, 不分成功是选项
    return pos == argument.length() ? false : true;
  } catch (const std::invalid_argument&) {
    // 不是数字格式，是选项
    return true;
  } catch (const std::out_of_range&) {
    // 数字太大溢出了，但仍然是数字格式，不是选项
    return false;
  }
  return false;
}
