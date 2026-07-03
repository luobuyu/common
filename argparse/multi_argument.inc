#pragma once
#include <iostream>
#include <limits>

#include "multi_argument.h"

namespace dry {
namespace argparse {

// 构造函数实现
template <typename T>
MultiArgument<T>::MultiArgument(const std::vector<std::string>& names,
                                std::vector<T>& target,
                                const std::string& description)
    : Argument(names, description),
      m_expected_min(1),  // 默认至少需要1个值
      m_expected_max(1)   // 默认最多1个值
{
  BindTo(target);
}

template <typename T>
MultiArgument<T>::MultiArgument(const ArgumentType& type,
                                const std::vector<std::string>& names,
                                std::vector<T>& target,
                                const std::string& description)
    : Argument(type, names, description),
      m_expected_min(1),  // 默认至少需要1个值
      m_expected_max(1)   // 默认最多1个值
{
  BindTo(target);
}

template <typename T>
MultiArgument<T>::MultiArgument(const std::vector<std::string>& names,
                                T& target, const std::string& description)
    : Argument(names, description),
      m_expected_min(1),  // 默认至少需要1个值
      m_expected_max(1)   // 默认最多1个值
{
  BindTo(target);
}

template <typename T>
MultiArgument<T>::MultiArgument(const ArgumentType& type,
                                const std::vector<std::string>& names,
                                T& target, const std::string& description)
    : Argument(type, names, description),
      m_expected_min(1),  // 默认至少需要1个值
      m_expected_max(1)   // 默认最多1个值
{
  BindTo(target);
}

template <typename T>
MultiArgument<T>::MultiArgument(const std::vector<std::string>& names,
                                const std::string& description)
    : Argument(names, description),
      m_expected_min(1),  // 默认至少需要1个值
      m_expected_max(1)   // 默认最多1个值
{}

template <typename T>
MultiArgument<T>::MultiArgument(const ArgumentType& type,
                                const std::vector<std::string>& names,
                                const std::string& description)
    : Argument(type, names, description),
      m_expected_min(1),  // 默认至少需要1个值
      m_expected_max(1)   // 默认最多1个值
{}

// 值操作实现
template <typename T>
const std::vector<T>& MultiArgument<T>::GetValues() const {
  if (!m_values.empty()) {
    return m_values;
  }
  if (m_default_values.has_value()) {
    return m_default_values.value();
  }
  static const std::vector<T> empty;
  return empty;
}

template <typename T>
void MultiArgument<T>::AddValue(const T& value) {
  m_values.emplace_back(value);
}

template <typename T>
void MultiArgument<T>::SetValues(const std::vector<T>& values) {
  m_values = values;
}

template <typename T>
void MultiArgument<T>::ClearValues() {
  m_values.clear();
}

template <typename T>
size_t MultiArgument<T>::ValueCount() const {
  return m_values.size();
}

// 默认值设置实现
template <typename T>
MultiArgument<T>& MultiArgument<T>::DefaultValue(const T& value) {
  m_default_values = std::vector<T>{value};
  return *this;
}

template <typename T>
MultiArgument<T>& MultiArgument<T>::DefaultValues(
    const std::vector<T>& values) {
  m_default_values = values;
  return *this;
}

template <typename T>
bool MultiArgument<T>::HasDefaultValue() const {
  return m_default_values.has_value();
}

template <typename T>
const std::vector<T>& MultiArgument<T>::GetDefaultValues() const {
  if (!m_default_values.has_value()) {
    static const std::vector<T> empty;
    return empty;
  }
  return m_default_values.value();
}

// Expected 控制实现
template <typename T>
MultiArgument<T>& MultiArgument<T>::Expected(int count) {
  if (count == static_cast<int>(ExpectedCount::Unlimited)) {
    // nargs='*': 0到无限
    m_expected_min = 0;
    m_expected_max = std::numeric_limits<int>::max();
  } else if (count == static_cast<int>(ExpectedCount::AtLeastOne)) {
    // nargs='+': 至少1个
    m_expected_min = 1;
    m_expected_max = std::numeric_limits<int>::max();
  } else if (count >= 0) {
    // nargs=N: 精确N个
    m_expected_min = count;
    m_expected_max = count;
  } else {
    std::cerr << "Invalid Expected count: " << count << std::endl;
    return *this;  // 错误时直接返回,保持链式调用
  }
  return *this;
}

// 枚举类型版本的 expected() - 委托给整数版本
template <typename T>
MultiArgument<T>& MultiArgument<T>::Expected(ExpectedCount count) {
  return Expected(static_cast<int>(count));
}

template <typename T>
MultiArgument<T>& MultiArgument<T>::Expected(int min, int max) {
  if (min < 0 || max < min) {
    std::cerr << "Invalid Expected range: " << min << ", " << max << std::endl;
    return *this;  // 错误时直接返回,保持链式调用
  }
  m_expected_min = min;
  m_expected_max = max;
  return *this;
}

template <typename T>
int MultiArgument<T>::GetExpectedMin() const {
  return m_expected_min;
}

template <typename T>
int MultiArgument<T>::GetExpectedMax() const {
  return m_expected_max;
}

// 外部变量绑定实现
template <typename T>
MultiArgument<T>& MultiArgument<T>::BindTo(std::vector<T>& target) {
  m_sync_to_target = [&target, this]() {
    if (!this->m_values.empty()) {
      target = this->m_values;
    } else if (this->m_default_values.has_value()) {
      target = this->m_default_values.value();
    }
  };
  return *this;
}

template <typename T>
MultiArgument<T>& MultiArgument<T>::BindTo(T& target) {
  m_sync_to_target = [&target, this]() {
    if (!this->m_values.empty()) {
      target = this->m_values.front();  // 绑定第一个值
    } else if (this->m_default_values.has_value() &&
               !this->m_default_values->empty()) {
      target = this->m_default_values->front();
    }
  };
  return *this;
}

// 验证器实现（只保留整体验证）
template <typename T>
MultiArgument<T>& MultiArgument<T>::Validator(
    std::function<bool(const std::vector<T>&)> validator) {
  m_validator = validator;
  return *this;
}

template <typename T>
bool MultiArgument<T>::IsValid() const {
  if (!m_validator) {
    return true;  // 没有验证器，默认有效
  }
  return m_validator(GetValues());
}

template <typename T>
void MultiArgument<T>::Validate() const {
  if (m_validator && !m_validator(m_values)) {
    throw std::runtime_error("Validation failed for argument: " +
                             this->GetNames().front());
  }
}

// 链式调用实现
template <typename T>
MultiArgument<T>& MultiArgument<T>::Description(
    const std::string& description) {
  SetDescription(description);
  return *this;
}

template <typename T>
MultiArgument<T>& MultiArgument<T>::Required() {
  SetRequired(true);
  return *this;
}

template <typename T>
MultiArgument<T>& MultiArgument<T>::Callback(std::function<void()> callback) {
  SetCallback(callback);
  return *this;
}

}  // namespace argparse
}  // namespace dry
