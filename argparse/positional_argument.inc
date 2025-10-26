#pragma once

#include "positional_argument.h"

// 构造函数1: 绑定 vector<T>
template <typename T>
PositionalArgument<T>::PositionalArgument(const std::vector<std::string>& names,
                                          std::vector<T>& target,
                                          const std::string& description,
                                          bool required,
                                          std::function<void()> callback)
    : MultiArgument<T>(names, description, required, callback) {
  this->bindTo(target);
}

// 构造函数2: 绑定单个 T
template <typename T>
PositionalArgument<T>::PositionalArgument(const std::vector<std::string>& names,
                                          T& target,
                                          const std::string& description,
                                          bool required,
                                          std::function<void()> callback)
    : MultiArgument<T>(names, description, required, callback) {
  this->bindTo(target);
}

// 构造函数3: 不绑定
template <typename T>
PositionalArgument<T>::PositionalArgument(const std::vector<std::string>& names,
                                          const std::string& description,
                                          bool required,
                                          std::function<void()> callback)
    : MultiArgument<T>(names, description, required, callback) {}

// 重写解析方法
template <typename T>
size_t PositionalArgument<T>::parse(const std::vector<std::string>& args,
                                    size_t current_index) {
  // 位置参数解析逻辑
  return MultiArgument<T>::parse(args, current_index);
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