#pragma once
#include "multi_argument.h"
#include <cctype>

// 判断字符串是否看起来像一个选项参数（以 '-' 开头，且不是负数）
inline bool isOptionString(const std::string& str) {
  if (str.size() < 2 || str[0] != '-') {
    return false;
  }
  
  // 长选项: --xxx
  if (str[1] == '-') {
    return true;
  }
  
  // 检查是否为负数（短选项或负数）
  // 如果第二个字符是数字，可能是负数 -123
  if (std::isdigit(static_cast<unsigned char>(str[1]))) {
    // 尝试解析为数字
    try {
      size_t pos = 0;
      std::stod(str, &pos);
      // 如果完全解析成功，说明是数字
      return pos != str.length();  // 如果有未解析部分，是选项
    } catch (const std::invalid_argument&) {
      return true;  // 不是数字格式，是选项
    } catch (const std::out_of_range&) {
      return false;  // 数字溢出，当作数字处理
    }
  }
  
  // -x 形式：是短选项
  return true;
}

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