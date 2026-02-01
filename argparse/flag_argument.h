#pragma once

#include "argument.h"
#include <optional>

class FlagArgument : public Argument {
 public:
  // 构造函数（不带 required 和 callback 参数，避免隐式转换问题）
  // 使用链式调用 .required() 和 .callback() 来设置
  FlagArgument(const std::vector<std::string>& names,
               const std::string& description = "");
  FlagArgument(const std::vector<std::string>& names, bool& target,
               const std::string& description = "");

  bool getFlag() const;
  void setFlag(bool value);
  void setDefaultValue(bool value);
  bool getDefaultValue() const;

  // 重写检查是否设置了默认值
  bool hasDefaultValue() const override;

  // 链式调用
  FlagArgument& description(const std::string& description);
  FlagArgument& required();
  FlagArgument& defaultValue(bool value);
  FlagArgument& callback(std::function<void()> callback);
  FlagArgument& bindTo(bool& target);

  // 验证器（对布尔值进行验证）
  FlagArgument& validator(std::function<bool(bool)> validator);
  // 重写基类验证器接口
  bool isValid() const override;
  void validate() const override;

  // 重写多态方法
  // FlagArgument 不需要额外参数，忽略 args 和 current_index
  size_t parse(const std::vector<std::string>& args, size_t current_index = 0) override;
  
  // 判断命令行参数是否匹配此标志参数
  bool matches(const std::string& arg) const override;

  // 验证 flag 参数名称（必须以 '-' 开头）
  void validateNames() const override;

 private:
  bool m_flag;     // 存储标志参数的状态，true表示启用，false表示禁用
  std::optional<bool> m_default_flag; // 默认值（使用 optional 判断是否设置）
  std::function<bool(bool)> m_validator;  // 验证器
};
