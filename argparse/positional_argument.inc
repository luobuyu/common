#pragma once

#include "positional_argument.h"
#include "../dry/dry_string.h"

// 构造函数1: 绑定 vector<T>
template <typename T>
PositionalArgument<T>::PositionalArgument(const std::vector<std::string>& names,
                                          std::vector<T>& target,
                                          const std::string& description,
                                          bool required,
                                          std::function<void()> callback)
    : MultiArgument<T>(ArgumentType::Positional, names, description, required, callback) {
  this->bindTo(target);
}

// 构造函数2: 绑定单个 T
template <typename T>
PositionalArgument<T>::PositionalArgument(const std::vector<std::string>& names,
                                          T& target,
                                          const std::string& description,
                                          bool required,
                                          std::function<void()> callback)
    : MultiArgument<T>(ArgumentType::Positional, names, description, required, callback) {
  this->bindTo(target);
}

// 构造函数3: 不绑定
template <typename T>
PositionalArgument<T>::PositionalArgument(const std::vector<std::string>& names,
                                          const std::string& description,
                                          bool required,
                                          std::function<void()> callback)
    : MultiArgument<T>(ArgumentType::Positional, names, description, required, callback) {}

// 重写解析方法
template <typename T>
size_t PositionalArgument<T>::parse(const std::vector<std::string>& args,
                                    size_t current_index) {
  size_t consumed = 0;
  int max_count = this->getExpectedMax();
  
  // 贪婪消耗参数，直到：
  // 1. 达到最大期望数量
  // 2. 遇到非位置参数
  // 3. 用完所有参数
  for (size_t i = current_index; i < args.size() && this->valueCount() < max_count; ++i) {
    const std::string& arg = args[i];
    // 遇到选项参数，停止消耗
    if (!matches(arg)) {
      break;
    }
    this->addValue(dry::strTo<T>(arg));
    consumed++;
  }
  
  // 检查是否满足最小期望数量
  int min_count = this->getExpectedMin();
  if (this->valueCount() < min_count) {
    throw std::runtime_error("Positional argument expects at least " + 
                             std::to_string(min_count) + " value(s), got " +
                             std::to_string(this->valueCount()));
  }
  
  // 标记为已解析
  this->setParsed(true);
  
  // 同步到外部变量 (如果绑定了)
  if (this->m_sync_to_target) {
    this->m_sync_to_target();
  }
  
  // 执行用户回调 (如果设置了)
  if (this->m_callback) {
    this->m_callback();
  }
  
  return consumed;
}

// 重写匹配方法
template <typename T>
bool PositionalArgument<T>::matches(const std::string& arg) const {
  // 位置参数匹配任何不以 '-' 开头的参数
  // 位置参数按顺序识别,不是按名称匹配
  return !arg.empty() && arg[0] != '-';
}

// 验证名称
template <typename T>
void PositionalArgument<T>::validateNames() const {
  const auto& names = this->getNames();
  
  if (names.empty()) {
    throw std::invalid_argument("Positional argument must have at least one name");
  }
  
  for (const auto& name : names) {
    if (name.empty()) {
      throw std::invalid_argument("Positional argument name cannot be empty");
    }
    if (name[0] == '-') {
      throw std::invalid_argument("Positional argument name cannot start with '-': " + name);
    }
  }
}