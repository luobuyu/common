#include "../dry_core/dry_string.h"
#include "option_argument.h"

namespace dry {
namespace argparse {

// 构造函数1: 绑定 vector<T>
template <typename T>
OptionArgument<T>::OptionArgument(const std::vector<std::string>& names,
                                  std::vector<T>& target,
                                  const std::string& description)
    : MultiArgument<T>(ArgumentType::Option, names, description) {
  this->bindTo(target);
}

// 构造函数2: 绑定单个 T
template <typename T>
OptionArgument<T>::OptionArgument(const std::vector<std::string>& names,
                                  T& target, const std::string& description)
    : MultiArgument<T>(ArgumentType::Option, names, description) {
  this->bindTo(target);
}

// 构造函数3: 不绑定
template <typename T>
OptionArgument<T>::OptionArgument(const std::vector<std::string>& names,
                                  const std::string& description)
    : MultiArgument<T>(ArgumentType::Option, names, description) {}

// 获取和设置值
template <typename T>
OptionArgument<T>& OptionArgument<T>::value(const T& value) {
  this->clearValues();
  this->addValue(value);
  return *this;
}
template <typename T>
OptionArgument<T>& OptionArgument<T>::description(
    const std::string& description) {
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
  MultiArgument<T>::defaultValue(value);  // 显式调用父类方法，避免递归
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
  // OptionArgument 解析逻辑：消耗后续参数直到遇到下一个选项或达到上限
  size_t consumed = 0;
  int max_count = this->getExpectedMax();
  bool terminated_by_dashdash = false;

  // 从 current_index + 1 开始逐个消耗参数
  for (size_t i = current_index + 1; i < args.size() && consumed < max_count;
       ++i) {
    const std::string& next_arg = args[i];
    // 如果遇到 "--"，表示用户想提前结束当前选项的参数收集
    if (next_arg == "--") {
      terminated_by_dashdash = true;
      break;
    }
    // 如果遇到选项参数，停止消耗
    if (isOptionString(next_arg)) {
      break;
    }
    this->addValue(dry::strTo<T>(next_arg));
    consumed++;
  }

  // 检查是否满足最小期望数量
  int min_count = this->getExpectedMin();
  if (this->valueCount() < min_count) {
    throw std::runtime_error("Option " + args[current_index] +
                             " requires at least " + std::to_string(min_count) +
                             " value(s), got " + std::to_string(consumed));
  }

  // 标记为已解析
  this->setParsed(true);

  // 同步到外部变量 (如果绑定了)
  if (this->m_sync_to_target) {
    this->m_sync_to_target();
  }

  // 自动调用验证器
  this->validate();

  // 执行用户回调
  if (this->m_callback) {
    this->m_callback();
  }

  // 消耗了选项参数本身 + 值的个数 + (如果被 -- 终止，还要消耗 --)
  return consumed + 1 + (terminated_by_dashdash ? 1 : 0);
}

// 重写匹配方法
template <typename T>
bool OptionArgument<T>::matches(const std::string& arg) const {
  if (arg.empty() || arg[0] != '-') {
    return false;  // 不是选项参数
  }
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
      throw std::invalid_argument("Option name cannot be empty");
    }
    if (name == "-" || name == "--") {
      throw std::invalid_argument("Option name cannot only be '-' or '--'");
    }
    if (name[0] != '-') {
      throw std::invalid_argument("Option name must start with '-': " + name);
    }
  }
}

}  // namespace argparse
}  // namespace dry