#pragma once

#include "argument.h"

class FlagArgument : public Argument {
 public:
  // 统一构造函数
  FlagArgument(const std::vector<std::string>& names,
               const std::string& description = "",
               bool required = false,
               std::function<void()> callback = nullptr);
  FlagArgument(const std::vector<std::string>& names, bool& target,
               const std::string& description = "",
               bool required = false, std::function<void()> callback = nullptr);

  bool getFlag() const;
  void setFlag(bool value);
  void setDefaultValue(bool value);
  bool getDefaultValue() const;

  // 链式调用
  FlagArgument& description(const std::string& description);
  FlagArgument& required();
  FlagArgument& defaultValue(bool value);
  FlagArgument& callback(std::function<void()> callback);
  FlagArgument& bindTo(bool& target);

  // 重写多态方法
  // FlagArgument 不需要额外参数，忽略 args 和 current_index
  size_t parse(const std::vector<std::string>& args, size_t current_index = 0) override;
  
  // 判断命令行参数是否匹配此标志参数
  bool matches(const std::string& arg) const override;

 private:
  bool m_flag;     // 存储标志参数的状态，true表示启用，false表示禁用
  bool m_default_flag; // 默认值
};
