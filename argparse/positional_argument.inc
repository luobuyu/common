#pragma once
#include <iostream>
#include <sstream>

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
