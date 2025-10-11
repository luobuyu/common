#pragma once
#include "argument.h"
#include <optional>

template <typename T>
class OptionArgument : public Argument {
 public:
  // 统一构造函数
  OptionArgument(const std::vector<std::string>& names,
                 const std::string& description,
                 T* target = nullptr,
                 bool required = false,
                 std::function<void()> callback = nullptr);

  const T& getValue() const;
  void setValue(const T& value);

  void setDefaultValue(const T& value);
  const T& getDefaultValue() const;
  bool hasDefaultValue() const;

  void setValidator(std::function<bool(const T&)> validator);
  bool isValid(const T& value) const;

  // 链式调用
  OptionArgument<T>& value(const T& value);
  OptionArgument<T>& description(const std::string& description);
  OptionArgument<T>& required();
  OptionArgument<T>& defaultValue(const T& value);
  OptionArgument<T>& validator(std::function<bool(const T&)> validator);
  OptionArgument<T>& callback(std::function<void()> callback);

  // 重写多态方法
  // OptionArgument 从 args[current_index+1] 获取值
  size_t parse(const std::vector<std::string>& args, size_t current_index = 0) override;
  
  // 判断命令行参数是否匹配此选项参数
  bool matches(const std::string& arg) const override;

 private:
  T m_value{};  // 存储选项参数的值
  std::optional<T> m_default_value; // 默认值
  T* m_target; // 指向一个变量，用于存储选项参数的值
  std::function<bool(const T&)> m_validator; // 选项参数的值的验证函数
};

#include "option_argument.inc"