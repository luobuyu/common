#include "flag_argument.h"
#include "option_argument.h"

// 使用默认参数的构造函数

FlagArgument::FlagArgument(const std::vector<std::string>& names,
                           const std::string& description, bool* target,
                           bool required, std::function<void()> callback)
  : Argument(names, description, required, callback), m_flag(false), m_target(target) {}

FlagArgument::FlagArgument(const std::vector<std::string>& names,
                           const std::string& description, bool required,
                           std::function<void()> callback)
  : Argument(names, description, required, callback), m_flag(false), m_target(nullptr) {}

bool FlagArgument::getFlag() const { return m_flag; }

void FlagArgument::setFlag(bool value) {
  m_flag = value;
  // 如果设置了目标变量，同时更新它
  if (m_target != nullptr) {
    *m_target = value;
  }
  // 如果设置了回调函数，调用它
  if(m_callback) {
    m_callback();
  }
}

FlagArgument& FlagArgument::flag(bool value) {
  setFlag(value);
  return *this;
}

FlagArgument& FlagArgument::description(const std::string& description) {
  setDescription(description);
  return *this;
}
