#pragma once
#include "argument.h"
#include <optional>

template <typename T>
class MultiArgument : public Argument {
public:
 // 类内枚举类：清晰的语义 + 类型安全
 enum class ExpectedCount : int {
   Unlimited = -1,  // nargs='*':  0 到无限个值
   AtLeastOne = -2  // nargs='+':  至少1个值
 };

 // 构造函数
 // 参数顺序：必需参数 → 常用可选参数
 // - names: 参数名称（必需）
 // - target: 绑定的外部变量（必需）
 // - description: 描述信息（可选）
 MultiArgument(const std::vector<std::string>& names, std::vector<T>& target,
               const std::string& description = "");
 MultiArgument(const ArgumentType& type, const std::vector<std::string>& names, std::vector<T>& target,
               const std::string& description = "");
 MultiArgument(const std::vector<std::string>& names,
               T& target, const std::string& description = "");
 MultiArgument(const ArgumentType& type, const std::vector<std::string>& names,
               T& target, const std::string& description = "");

 // 构造函数（不绑定）
 // - names: 参数名称（必需）
 // - description: 描述信息（可选）
 MultiArgument(const std::vector<std::string>& names,
               const std::string& description = "");
 MultiArgument(const ArgumentType& type, const std::vector<std::string>& names,
               const std::string& description = "");
 // 值操作
 const std::vector<T>& getValues() const;
 void addValue(const T& value);
 void setValues(const std::vector<T>& values);
 void clearValues();
 size_t valueCount() const;

 // 默认值设置（统一使用 std::optional<std::vector<T>>）
 MultiArgument<T>& defaultValue(const T& value);  // 单个默认值
 MultiArgument<T>& defaultValues(
     const std::vector<T>& values);  // 多个默认值
 bool hasDefaultValue() const override;  // 重写基类虚方法
 const std::vector<T>& getDefaultValues() const;

 // Expected 参数数量控制
 MultiArgument<T>& expected(int count);  // 精确数量或特殊值（整数）
 MultiArgument<T>& expected(ExpectedCount count);  // 枚举类型：强类型安全
 MultiArgument<T>& expected(int min, int max);     // 范围控制
 int getExpectedMin() const;
 int getExpectedMax() const;

 // 外部变量绑定
 MultiArgument<T>& bindTo(std::vector<T>& target);
 MultiArgument<T>& bindTo(T& target);

     // 验证器（只保留整体验证）
MultiArgument<T>& validator(
    std::function<bool(const std::vector<T>&)> validator);
bool isValid() const override;
// 验证并抛出异常（供 parse 时调用）
void validate() const override;

 // 链式调用
 MultiArgument<T>& description(const std::string& description);
 MultiArgument<T>& required();
 MultiArgument<T>& callback(std::function<void()> callback);

 // 重写多态方法
 // MultiArgument 接收值列表（二阶段模式），忽略 current_index
 // args: 专门给这个位置参数的值列表
 size_t parse(const std::vector<std::string>& args,
              size_t current_index = 0) override = 0;

 // 判断命令行参数是否匹配此位置参数
 // 位置参数匹配任何不以 '-' 开头的参数
 bool matches(const std::string& arg) const override = 0;

private:
 std::vector<T> m_values;                         // 已解析的值（统一存储）
 std::optional<std::vector<T>> m_default_values;  // 是否存在默认值

 int m_expected_min;  // 最小期望数量
 int m_expected_max;  // 最大期望数量

 std::function<bool(const std::vector<T>&)> m_validator;  // 整体验证器
};

#include "multi_argument.inc"