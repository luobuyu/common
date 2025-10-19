#pragma once
#include <iostream>

#include "multi_argument.h"

// 构造函数实现
template <typename T>
MultiArgument<T>::MultiArgument(const std::vector<std::string>& names,
                                std::vector<T>& target,
                                const std::string& description, bool required,
                                std::function<void()> callback)
    : Argument(names, description, required, callback),
      m_expected_min(1),  // 默认至少需要1个值
      m_expected_max(1)   // 默认最多1个值
{
  bindTo(target);
}

template <typename T>
MultiArgument<T>::MultiArgument(const std::vector<std::string>& names,
                                T& target, const std::string& description,
                                bool required, std::function<void()> callback)
    : Argument(names, description, required, callback),
      m_expected_min(1),  // 默认至少需要1个值
      m_expected_max(1)   // 默认最多1个值
{
  bindTo(target);
}

template <typename T>
MultiArgument<T>::MultiArgument(const std::vector<std::string>& names,
                                const std::string& description, bool required,
                                std::function<void()> callback)
    : Argument(names, description, required, callback),
      m_expected_min(1),  // 默认至少需要1个值
      m_expected_max(1)   // 默认最多1个值
{}

// 值操作实现
template <typename T>
const std::vector<T>& MultiArgument<T>::getValues() const {
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
void MultiArgument<T>::addValue(const T& value) {
  m_values.emplace_back(value);
}

template <typename T>
void MultiArgument<T>::setValues(const std::vector<T>& values) {
  m_values = values;
}

template <typename T>
void MultiArgument<T>::clearValues() {
  m_values.clear();
}

template <typename T>
size_t MultiArgument<T>::valueCount() const {
  return m_values.size();
}

// 默认值设置实现
template <typename T>
MultiArgument<T>& MultiArgument<T>::defaultValue(const T& value) {
  m_default_values = std::vector<T>{value};
  return *this;
}

template <typename T>
MultiArgument<T>& MultiArgument<T>::defaultValues(
    const std::vector<T>& values) {
  m_default_values = values;
  return *this;
}

template <typename T>
bool MultiArgument<T>::hasDefaultValues() const {
  return m_default_values.has_value();
}

template <typename T>
const std::vector<T>& MultiArgument<T>::getDefaultValues() const {
  if (!m_default_values.has_value()) {
    static const std::vector<T> empty;
    return empty;
  }
  return m_default_values.value();
}

// Expected 控制实现
template <typename T>
MultiArgument<T>& MultiArgument<T>::expected(int count) {
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
    std::cerr << "Invalid expected count: " << count << std::endl;
    return *this;  // 错误时直接返回,保持链式调用
  }
  return *this;
}

// 枚举类型版本的 expected() - 委托给整数版本
template <typename T>
MultiArgument<T>& MultiArgument<T>::expected(ExpectedCount count) {
  return expected(static_cast<int>(count));
}

template <typename T>
MultiArgument<T>& MultiArgument<T>::expected(int min, int max) {
  if (min < 0 || max < min) {
    std::cerr << "Invalid expected range: " << min << ", " << max << std::endl;
    return *this;  // 错误时直接返回,保持链式调用
  }
  m_expected_min = min;
  m_expected_max = max;
  return *this;
}

template <typename T>
int MultiArgument<T>::getExpectedMin() const {
  return m_expected_min;
}

template <typename T>
int MultiArgument<T>::getExpectedMax() const {
  return m_expected_max;
}

// 外部变量绑定实现
template <typename T>
MultiArgument<T>& MultiArgument<T>::bindTo(std::vector<T>& target) {
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
MultiArgument<T>& MultiArgument<T>::bindTo(T& target) {
  m_sync_to_target = [&target, this]() {
    if (!this->m_values.empty()) {
      target = this->m_values.front();  // 绑定第一个值
    } else if (this->m_default_values.has_value() &&
               !this->m_default_values->empty()) {
      target = this->m_default_values->back();  // 绑定最后一个默认值
    }
  };
  return *this;
}

// 验证器实现（只保留整体验证）
template <typename T>
MultiArgument<T>& MultiArgument<T>::validator(
    std::function<bool(const std::vector<T>&)> validator) {
  m_validator = validator;
  return *this;
}

template <typename T>
bool MultiArgument<T>::isValid() const {
  if (!m_validator) {
    return true;  // 没有验证器，默认有效
  }
  return m_validator(getValues());
}

// 链式调用实现
template <typename T>
MultiArgument<T>& MultiArgument<T>::description(
    const std::string& description) {
  setDescription(description);
  return *this;
}

template <typename T>
MultiArgument<T>& MultiArgument<T>::required() {
  setRequired(true);
  return *this;
}

template <typename T>
MultiArgument<T>& MultiArgument<T>::callback(
    std::function<void()> callback) {
  setCallback(callback);
  return *this;
}
