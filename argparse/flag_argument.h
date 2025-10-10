#pragma once

#include "argument.h"

class FlagArgument : public Argument {
 public:
  // 统一构造函数
  FlagArgument(const std::vector<std::string>& names,
               const std::string& description,
               bool* target = nullptr,
               bool required = false,
               std::function<void()> callback = nullptr);

  bool getFlag() const;
  void setFlag(bool value);
  void setDefaultValue(bool value);
  bool getDefaultValue() const;

  // 链式调用
  FlagArgument& flag(bool value);
  FlagArgument& description(const std::string& description);
  FlagArgument& required();
  FlagArgument& defaultValue(bool value);
  FlagArgument& callback(std::function<void()> callback);


 private:
  bool m_flag;     // 存储标志参数的状态，true表示启用，false表示禁用
  bool m_default_flag; // 默认值
  bool* m_target;  // 指向一个bool变量，用于存储标志参数的状态
};
