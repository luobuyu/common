#include "flag_argument.h"

#include <algorithm>
#include <stdexcept>

namespace dry {
namespace argparse {

// 构造函数（不带 required 和 callback 参数，避免隐式转换问题）
FlagArgument::FlagArgument(const std::vector<std::string>& names,
                           const std::string& Description)
    : Argument(ArgumentType::Flag, names, Description), m_flag(false) {}

FlagArgument::FlagArgument(const std::vector<std::string>& names, bool& target,
                           const std::string& Description)
    : Argument(ArgumentType::Flag, names, Description), m_flag(false) {
  BindTo(target);
}

bool FlagArgument::GetFlag() const {
  // 简化逻辑:
  // - 如果被解析过(m_parsed=true)，返回 m_flag
  // - 否则返回默认值（如果有），或者 false
  if (IsParsed()) {
    return m_flag;
  }
  return m_default_flag.value_or(false);
}

void FlagArgument::SetFlag(bool value) {
  m_flag = value;
  SetParsed(true);  // 标记为已解析

  // 同步到外部变量 (如果绑定了)
  if (m_sync_to_target) {
    m_sync_to_target();
  }

  // 执行用户回调 (如果设置了)
  if (m_callback) {
    m_callback();
  }
}

void FlagArgument::SetDefaultValue(bool value) { m_default_flag = value; }

bool FlagArgument::GetDefaultValue() const {
  return m_default_flag.value_or(false);
}

bool FlagArgument::HasDefaultValue() const {
  return m_default_flag.has_value();
}

FlagArgument& FlagArgument::Description(const std::string& Description) {
  SetDescription(Description);
  return *this;
}

FlagArgument& FlagArgument::Required() {
  SetRequired(true);
  return *this;
}

FlagArgument& FlagArgument::DefaultValue(bool value) {
  SetDefaultValue(value);
  return *this;
}

FlagArgument& FlagArgument::Callback(std::function<void()> Callback) {
  SetCallback(Callback);
  return *this;
}

FlagArgument& FlagArgument::BindTo(bool& target) {
  m_sync_to_target = [&target, this]() { target = this->GetFlag(); };
  return *this;
}

FlagArgument& FlagArgument::Validator(std::function<bool(bool)> Validator) {
  m_validator = Validator;
  return *this;
}

bool FlagArgument::IsValid() const {
  // 如果没有设置验证器，默认验证通过
  if (!m_validator) {
    return true;
  }
  return m_validator(GetFlag());
}

void FlagArgument::Validate() const {
  if (!IsValid()) {
    std::string name = GetNames().empty() ? "unknown" : GetNames()[0];
    throw std::invalid_argument("Validation failed for flag argument: " + name);
  }
}

size_t FlagArgument::Parse(const std::vector<std::string>& args,
                           size_t current_index) {
  SetFlag(true);
  Validate();  // 解析后自动验证
  return 1;    // 消耗了标志参数本身
}

bool FlagArgument::Matches(const std::string& arg) const {
  // 检查 arg 是否在名称列表中
  if (arg.empty() || arg[0] != '-') {
    return false;  // 不是选项参数
  }
  const auto& names = GetNames();
  return std::find(names.begin(), names.end(), arg) != names.end();
}

void FlagArgument::ValidateNames() const {
  const auto& names = GetNames();

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
      throw std::invalid_argument("Flag name must Start with '-': " + name);
    }
  }
}

}  // namespace argparse
}  // namespace dry
