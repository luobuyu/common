#pragma once

#include <optional>

#include "argument.h"

namespace dry {
namespace argparse {

class FlagArgument : public Argument {
 public:
  // 构造函数（不带 required 和 callback 参数，避免隐式转换问题）
  // 使用链式调用 .required() 和 .callback() 来设置
  FlagArgument(const std::vector<std::string>& names, const std::string& description = "");
  FlagArgument(const std::vector<std::string>& names, bool& target,
               const std::string& description = "");

  bool GetFlag() const;
  void SetFlag(bool value);
  void SetDefaultValue(bool value);
  bool GetDefaultValue() const;

  // 重写检查是否设置了默认值
  bool HasDefaultValue() const override;

  // 链式调用
  FlagArgument& Description(const std::string& description);
  FlagArgument& Required();
  FlagArgument& DefaultValue(bool value);
  FlagArgument& Callback(std::function<void()> callback);
  FlagArgument& BindTo(bool& target);

  // 验证器（对布尔值进行验证）
  FlagArgument& Validator(std::function<bool(bool)> validator);
  // 重写基类验证器接口
  bool IsValid() const override;
  void Validate() const override;

  // 重写多态方法
  // FlagArgument 不需要额外参数，忽略 args 和 current_index
  size_t Parse(const std::vector<std::string>& args, size_t current_index = 0) override;

  // 判断命令行参数是否匹配此标志参数
  bool Matches(const std::string& arg) const override;

  // 验证 flag 参数名称（必须以 '-' 开头）
  void ValidateNames() const override;

 private:
  bool m_flag;  // 存储标志参数的状态，true表示启用，false表示禁用
  std::optional<bool> m_default_flag;     // 默认值（使用 optional 判断是否设置）
  std::function<bool(bool)> m_validator;  // 验证器
};

}  // namespace argparse
}  // namespace dry
