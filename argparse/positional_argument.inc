#pragma once
#include "positional_argument.h"

// 构造函数实现
template <typename T>
PositionalArgument<T>::PositionalArgument(
    const std::vector<std::string>& names,
    const std::string& description,
    std::vector<T>* target,
    bool required,
    std::function<void()> callback)
    : Argument(names, description, required, callback),
      m_vector_target(target),
      m_expected_min(1),  // 默认至少需要1个值
      m_expected_max(1)   // 默认最多1个值
{
}

template <typename T>
PositionalArgument<T>::PositionalArgument(
    const std::vector<std::string>& names,
    const std::string& description,
    bool required,
    std::function<void()> callback)
    : PositionalArgument(names, description, nullptr, required, callback)
{
}

// 值操作实现
template <typename T>
const std::vector<T>& PositionalArgument<T>::getValues() const {
  if (!m_values.empty()) {
    return m_values;  // 返回已解析的值
  }
  if (m_default_values.has_value()) {
    return m_default_values.value();  // 返回完整默认值（all-or-nothing）
  }
  static const std::vector<T> empty;
  return empty;  // 返回空vector
}

template <typename T>
void PositionalArgument<T>::addValue(const T& value) {
  m_values.push_back(value);
}

template <typename T>
void PositionalArgument<T>::setValues(const std::vector<T>& values) {
  m_values = values;
}

template <typename T>
void PositionalArgument<T>::clearValues() {
  m_values.clear();
}

template <typename T>
size_t PositionalArgument<T>::valueCount() const {
  return m_values.size();
}

// 默认值设置实现（方案C）
template <typename T>
PositionalArgument<T>& PositionalArgument<T>::defaultValue(const T& value) {
  m_default_values = std::vector<T>{value};
  return *this;
}

template <typename T>
PositionalArgument<T>& PositionalArgument<T>::defaultValues(const std::vector<T>& values) {
  m_default_values = values;
  return *this;
}

template <typename T>
bool PositionalArgument<T>::hasDefaultValues() const {
  return m_default_values.has_value();
}

template <typename T>
const std::vector<T>& PositionalArgument<T>::getDefaultValues() const {
  if (!m_default_values.has_value()) {
    static const std::vector<T> empty;
    return empty;
  }
  return m_default_values.value();
}

// Expected 控制实现
template <typename T>
PositionalArgument<T>& PositionalArgument<T>::expected(int count) {
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
    throw std::invalid_argument("Invalid expected count");
  }
  return *this;
}

// 枚举类型版本的 expected() - 委托给整数版本
template <typename T>
PositionalArgument<T>& PositionalArgument<T>::expected(ExpectedCount count) {
  return expected(static_cast<int>(count));
}

template <typename T>
PositionalArgument<T>& PositionalArgument<T>::expected(int min, int max) {
  if (min < 0 || max < min) {
    throw std::invalid_argument("Invalid expected range");
  }
  m_expected_min = min;
  m_expected_max = max;
  return *this;
}

template <typename T>
int PositionalArgument<T>::getExpectedMin() const {
  return m_expected_min;
}

template <typename T>
int PositionalArgument<T>::getExpectedMax() const {
  return m_expected_max;
}

// 外部变量绑定实现
template <typename T>
PositionalArgument<T>& PositionalArgument<T>::target(std::vector<T>& external_vector) {
  m_vector_target = &external_vector;
  return *this;
}

// 验证器实现（只保留整体验证）
template <typename T>
PositionalArgument<T>& PositionalArgument<T>::validator(
    std::function<bool(const std::vector<T>&)> validator) {
  m_validator = validator;
  return *this;
}

template <typename T>
bool PositionalArgument<T>::isValid() const {
  if (!m_validator) {
    return true;  // 没有验证器，默认有效
  }
  return m_validator(getValues());
}

// 链式调用实现
template <typename T>
PositionalArgument<T>& PositionalArgument<T>::description(const std::string& description) {
  setDescription(description);
  return *this;
}

template <typename T>
PositionalArgument<T>& PositionalArgument<T>::required() {
  setRequired(true);
  return *this;
}

template <typename T>
PositionalArgument<T>& PositionalArgument<T>::callback(std::function<void()> callback) {
  setCallback(callback);
  return *this;
}
