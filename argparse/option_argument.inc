#include "option_argument.h"
#include <sstream>

// 构造函数1: 绑定 vector<T>
template <typename T>
OptionArgument<T>::OptionArgument(const std::vector<std::string>& names,
                                  std::vector<T>& target,
                                  const std::string& description,
                                  bool required, std::function<void()> callback)
    : MultiArgument<T>(names, description, required, callback) {
  this->bindTo(target);
}

// 构造函数2: 绑定单个 T
template <typename T>
OptionArgument<T>::OptionArgument(const std::vector<std::string>& names,
                                  T& target, const std::string& description,
                                  bool required, std::function<void()> callback)
    : MultiArgument<T>(names, description, required, callback) {
  this->bindTo(target);
}

// 构造函数3: 不绑定
template <typename T>
OptionArgument<T>::OptionArgument(const std::vector<std::string>& names,
                                 const std::string& description,
                                 bool required, std::function<void()> callback)
    : MultiArgument<T>(names, description, required, callback) {}

// 获取和设置值
template <typename T>
OptionArgument<T>& OptionArgument<T>::value(const T& value) {
  this->clearValues();
  this->addValue(value);
  return *this;
}
template <typename T>
OptionArgument<T>& OptionArgument<T>::description(const std::string& description) {
  this->setDescription(description);
  return *this;
}
template <typename T>
OptionArgument<T>& OptionArgument<T>::required() {
  this->setRequired(true);
  return *this;
}
template <typename T>
OptionArgument<T>& OptionArgument<T>::defaultValue(const T& value) {
  this->defaultValue(value);
  return *this;
}
template <typename T>
OptionArgument<T>& OptionArgument<T>::callback(std::function<void()> callback) {
  this->setCallback(callback);
  return *this;
}

// 重写解析方法
template <typename T>
size_t OptionArgument<T>::parse(const std::vector<std::string>& args,
                                size_t current_index) {
  // 1. 检查是否有足够的参数
  if (current_index + 1 >= args.size()) {
    throw std::runtime_error("Option argument requires a value: " + args[current_index]);
  }
  const std::string& value_str = args[current_index + 1];
  // 2. 解析值
  std::istringstream iss(value_str);
  T value;
  if (!(iss >> value)) {
    throw std::runtime_error("Invalid value for option argument: " + value_str);
  }
  this->addValue(value);
  
  // 3. 标记为已解析
  this->setParsed(true);
  
  // 4. 同步到外部变量 (如果绑定了)
  if (this->m_sync_to_target) {
    this->m_sync_to_target();
  }
  
  // 5. 执行用户回调 (如果设置了)
  if (this->m_callback) {
    this->m_callback();
  }
  
  return 1;  // 消耗了1个参数 (value_str)
}

// 重写匹配方法
template <typename T>
bool OptionArgument<T>::matches(const std::string& arg) const {
  // 检查 arg 是否在名称列表中
  const auto& names = this->getNames();
  return std::find(names.begin(), names.end(), arg) != names.end();
}

// 验证名称
template <typename T>
void OptionArgument<T>::validateNames() const {
  const auto& names = this->getNames();
  
  if (names.empty()) {
    throw std::invalid_argument("Option argument must have at least one name");
  }

  for (const auto& name : names) {
    if (name.empty()) {
      throw std::invalid_argument("Flag name cannot be empty");
    }
    if (name[0] != '-' || name.size() > 1 && name[1] != '-') {
      throw std::invalid_argument("Flag name must start with '-' or '--': " +
                                  name);
    }
  }
}