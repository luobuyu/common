#include "option_argument.h"
#include <sstream>

template <typename T>
OptionArgument<T>::OptionArgument(const std::vector<std::string>& names,
                                  const std::string& description,
                                  T* target,
                                  bool required,
                                  std::function<void()> callback)
    : Argument(names, description, required, callback), m_target(target) {}

template <typename T>
const T& OptionArgument<T>::getValue() const {
  // 简化逻辑:
  // - 如果被解析过(m_parsed=true)，返回 m_value
  // - 否则返回默认值(如果有)
  // - 都没有则返回默认构造的值
  if (isParsed()) {
    return m_value;
  }
  if (m_default_value.has_value()) {
    return m_default_value.value();
  }
  return m_value;  // 返回默认构造的值
}

template <typename T>
void OptionArgument<T>::setValue(const T& value) {
  m_value = value;
  setParsed(true);  // 标记为已解析
  // 如果绑定了外部变量,同步更新
  if (m_target) {
    *m_target = value;
  }
}

template <typename T>
void OptionArgument<T>::setDefaultValue(const T& value) {
  m_default_value = value;
}

template <typename T>
const T& OptionArgument<T>::getDefaultValue() const {
  return m_default_value.value();
}

template <typename T>
bool OptionArgument<T>::hasDefaultValue() const {
  return m_default_value.has_value();
}

template <typename T>
void OptionArgument<T>::setValidator(std::function<bool(const T&)> validator) {
  m_validator = validator;
}

template <typename T>
bool OptionArgument<T>::isValid(const T& value) const {
  if (!m_validator) {
    return true;  // 没有验证器时默认有效
  }
  return m_validator(value);
}

template <typename T>
OptionArgument<T>& OptionArgument<T>::value(const T& value) {
  setValue(value);
  return *this;
}

template <typename T>
OptionArgument<T>& OptionArgument<T>::description(const std::string& description) {
  setDescription(description);
  return *this;
}

template <typename T>
OptionArgument<T>& OptionArgument<T>::required() {
  setRequired(true);
  return *this;
}

template <typename T>
OptionArgument<T>& OptionArgument<T>::defaultValue(const T& value) {
  setValue(value);
  setDefaultValue(value);
  return *this;
}

template <typename T>
OptionArgument<T>& OptionArgument<T>::validator(
    std::function<bool(const T&)> validator) {
  setValidator(validator);
  return *this;
}

template <typename T>
OptionArgument<T>& OptionArgument<T>::callback(std::function<void()> callback) {
  setCallback(callback);
  return *this;
}

template <typename T>
size_t OptionArgument<T>::parse(const std::vector<std::string>& args, size_t current_index) {
  // Option 需要一个额外的值
  if (current_index + 1 >= args.size()) {
    throw std::runtime_error("Expected value after " + args[current_index]);
  }
  
  // 从字符串转换为 T 类型
  // 这里需要实现字符串到 T 的转换
  const std::string& value_str = args[current_index + 1];
  
  // 使用 stringstream 进行转换
  std::istringstream iss(value_str);
  T value;
  if (!(iss >> value) || !iss.eof()) {
    throw std::runtime_error("Invalid value for " + args[current_index] + ": " + value_str);
  }
  
  setValue(value);
  return 1;  // 返回 1 表示消耗了 1 个额外参数
}

template <typename T>
bool OptionArgument<T>::matches(const std::string& arg) const {
  // 检查 arg 是否在名称列表中
  if(arg.empty() || arg[0] != '-') {
    return false; // 不是选项参数
  }
  const auto& names = getNames();
  return std::find(names.begin(), names.end(), arg) != names.end();
}