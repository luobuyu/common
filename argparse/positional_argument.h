#pragma once
#include "multi_argument.h"

// 方案：类内枚举 + 命名空间别名（推荐）
// 既保持封装性，又提供便捷访问
template <typename T>
class PositionalArgument : public MultiArgument<T> {
public:

  // 构造函数1: 绑定 vector<T>
 PositionalArgument(const std::vector<std::string>& names,
                    std::vector<T>& target, const std::string& description = "");

 // 构造函数2: 绑定单个 T
 PositionalArgument(const std::vector<std::string>& names, T& target,
                    const std::string& description = "");

 // 构造函数3: 不绑定
 PositionalArgument(const std::vector<std::string>& names,
                    const std::string& description = "");

 // 重写解析方法
 size_t parse(const std::vector<std::string>& args,
              size_t current_index = 0) override;

 // 重写匹配方法
 bool matches(const std::string& arg) const override;

 // 验证 positional 参数名称（不能以 '-' 开头）
 void validateNames() const override;
};

#include "positional_argument.inc"