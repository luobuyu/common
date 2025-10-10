#include "flag_argument.h"

// 统一构造函数
FlagArgument::FlagArgument(const std::vector<std::string>& names,
                           const std::string& description,
                           bool* target,
                           bool required,
                           std::function<void()> callback)
  : Argument(names, description, required, callback),
    m_flag(false),
    m_default_flag(false),
    m_target(target) {}

bool FlagArgument::getFlag() const {
  // 简化逻辑: 
  // - 如果被解析过(m_parsed=true)，返回 m_flag
  // - 否则返回默认值 m_default_flag
  if (isParsed()) {
    return m_flag;
  }
  return m_default_flag;
}

void FlagArgument::setFlag(bool value) {
  m_flag = value;
  setParsed(true);  // 标记为已解析
  // 如果设置了目标变量，同时更新它
  if (m_target != nullptr) {
    *m_target = value;
  }
  // 如果设置了回调函数，调用它
  if(m_callback) {
    m_callback();
  }
}

void FlagArgument::setDefaultValue(bool value) {
  m_default_flag = value;
}

bool FlagArgument::getDefaultValue() const {
  return m_default_flag;
}

FlagArgument& FlagArgument::flag(bool value) {
  setFlag(value);
  return *this;
}

FlagArgument& FlagArgument::description(const std::string& description) {
  setDescription(description);
  return *this;
}

FlagArgument& FlagArgument::required() {
  setRequired(true);
  return *this;
}

FlagArgument& FlagArgument::defaultValue(bool value) {
  setDefaultValue(value);
  return *this;
}

FlagArgument& FlagArgument::callback(std::function<void()> callback) {
  setCallback(callback);
  return *this;
}
