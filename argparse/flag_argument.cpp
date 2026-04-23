#include "flag_argument.h"

#include <algorithm>
#include <stdexcept>

namespace dry {
namespace argparse {

// 构造函数（不带 required 和 callback 参数，避免隐式转换问题）
FlagArgument::FlagArgument(const std::vector<std::string>& names,
                           const std::string& description)
    : Argument(ArgumentType::Flag, names, description), m_flag(false) {}

FlagArgument::FlagArgument(const std::vector<std::string>& names, bool& target,
                           const std::string& description)
    : Argument(ArgumentType::Flag, names, description), m_flag(false) {
  bindTo(target);
}

bool FlagArgument::getFlag() const {
  // 简化逻辑:
  // - 如果被解析过(m_parsed=true)，返回 m_flag
  // - 否则返回默认值（如果有），或者 false
  if (isParsed()) {
    return m_flag;
  }
  return m_default_flag.value_or(false);
}

void FlagArgument::setFlag(bool value) {
  m_flag = value;
  setParsed(true);  // 标记为已解析

  // 同步到外部变量 (如果绑定了)
  if (m_sync_to_target) {
    m_sync_to_target();
  }

  // 执行用户回调 (如果设置了)
  if (m_callback) {
    m_callback();
  }
}

void FlagArgument::setDefaultValue(bool value) { m_default_flag = value; }

bool FlagArgument::getDefaultValue() const {
  return m_default_flag.value_or(false);
}

bool FlagArgument::hasDefaultValue() const {
  return m_default_flag.has_value();
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

FlagArgument& FlagArgument::bindTo(bool& target) {
  m_sync_to_target = [&target, this]() { target = this->getFlag(); };
  return *this;
}

FlagArgument& FlagArgument::validator(std::function<bool(bool)> validator) {
  m_validator = validator;
  return *this;
}

bool FlagArgument::isValid() const {
  // 如果没有设置验证器，默认验证通过
  if (!m_validator) {
    return true;
  }
  return m_validator(getFlag());
}

void FlagArgument::validate() const {
  if (!isValid()) {
    std::string name = getNames().empty() ? "unknown" : getNames()[0];
    throw std::invalid_argument("Validation failed for flag argument: " + name);
  }
}

size_t FlagArgument::parse(const std::vector<std::string>& args,
                           size_t current_index) {
  setFlag(true);
  validate();  // 解析后自动验证
  return 1;    // 消耗了标志参数本身
}

bool FlagArgument::matches(const std::string& arg) const {
  // 检查 arg 是否在名称列表中
  if (arg.empty() || arg[0] != '-') {
    return false;  // 不是选项参数
  }
  const auto& names = getNames();
  return std::find(names.begin(), names.end(), arg) != names.end();
}

void FlagArgument::validateNames() const {
  const auto& names = getNames();

  if (names.empty()) {
    throw std::invalid_argument("Flag argument must have at least one name");
  }

  for (const auto& name : names) {
    if (name.empty()) {
      throw std::invalid_argument("Flag name cannot be empty");
    }
    if (name == "-" || name == "--") {
      throw std::invalid_argument("Flag name cannot only be '-' or '--'");
    }
    if (name[0] != '-') {
      throw std::invalid_argument("Flag name must start with '-': " + name);
    }
  }
}

}  // namespace argparse
}  // namespace dry
