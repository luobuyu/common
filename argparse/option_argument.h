#pragma once
#include "argument.h"

template <typename T>
class OptionArgument : public Argument {
 public:
  // 使用默认参数的构造函数
  OptionArgument(const std::vector<std::string>& names,
                 const std::string& description, T* target = nullptr,
                 bool required = false, std::function<void()> callback = nullptr);
  OptionArgument(const std::vector<std::string>& names,
                 const std::string& description, bool required = false,
                 std::function<void()> callback = nullptr);

  const T& getValue() const;
  void setValue(const T& value);
  void setDefaultValue(const T& value);
  T getDefaultValue() const;
  void setValidator(std::function<void()> validator);

  // 链式调用
  OptionArgument<T>& value(const T& value);
  OptionArgument<T>& description(const std::string& description);
  OptionArgument<T>& required();
  OptionArgument<T>& defaultValue(const T& value);
  OptionArgument<T>& validator(std::function<void()> validator);

 private:
  T m_value;  // 存储选项参数的值
  T m_default_value; // 默认值
  T* m_target; // 指向一个变量，用于存储选项参数的值
  std::function<void()> m_validator; // 选项参数的值的验证函数
};

#include "option_argument.inc"