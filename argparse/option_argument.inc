#include "option_argument.h"

template <typename T>
OptionArgument<T>::OptionArgument(const std::vector<std::string>& names,
                                  const std::string& description, T* target,
                                  bool required,
                                  std::function<void()> callback)
    : Argument(names, description, required, callback), m_target(target) {}

template <typename T>
OptionArgument<T>::OptionArgument(const std::vector<std::string>& names,
                                  const std::string& description, bool required,
                                  std::function<void()> callback) {}

template <typename T>
const T& OptionArgument<T>::getValue() const {
  return *m_target;
}

template <typename T>
void OptionArgument<T>::setValue(const T& value) {
  *m_target = value;
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