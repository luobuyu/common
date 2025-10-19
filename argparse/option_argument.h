#pragma once
#include "multi_argument.h"

template <typename T>
class OptionArgument : public MultiArgument<T> {
 public:
  // 构造函数1: 绑定 vector<T>
  OptionArgument(const std::vector<std::string>& names, std::vector<T>& target,
                 const std::string& description = "",
                 bool required = false,
                 std::function<void()> callback = nullptr);

  // 构造函数2: 绑定单个 T
  OptionArgument(const std::vector<std::string>& names, T& target,
                 const std::string& description = "",
                 bool required = false,
                 std::function<void()> callback = nullptr);

  // 构造函数3: 不绑定
  OptionArgument(const std::vector<std::string>& names,
                 const std::string& description = "",
                 bool required = false,
                 std::function<void()> callback = nullptr);
  
  // 链式调用
  OptionArgument<T>& value(const T& value);
  OptionArgument<T>& description(const std::string& description);
  OptionArgument<T>& required();
  OptionArgument<T>& defaultValue(const T& value);
  OptionArgument<T>& callback(std::function<void()> callback);

  // 重写多态方法
  // OptionArgument 需要一个额外参数，解析 args[current_index+1]
  size_t parse(const std::vector<std::string>& args,
               size_t current_index = 0) override;

  // 判断命令行参数是否匹配此选项参数
  bool matches(const std::string& arg) const override;

  // 验证 option 参数名称（必须以 '-' 开头）
  void validateNames() const override;
};

#include "option_argument.inc"