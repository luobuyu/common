#pragma once
#include "argument.h"
#include <optional>
#include <vector>
#include <functional>

// 方案：类内枚举 + 命名空间别名（推荐）
// 既保持封装性，又提供便捷访问
template <typename T>
class PositionalArgument : public Argument {
public:
  // 类内枚举类：清晰的语义 + 类型安全
  enum class ExpectedCount : int {
    Unlimited = -1,      // nargs='*':  0 到无限个值
    AtLeastOne = -2      // nargs='+':  至少1个值
  };

  // 统一构造函数
  PositionalArgument(const std::vector<std::string>& names,
                     const std::string& description,
                     std::vector<T>* target = nullptr,
                     bool required = false,
                     std::function<void()> callback = nullptr);

  // 值操作
  const std::vector<T>& getValues() const;
  void addValue(const T& value);
  void setValues(const std::vector<T>& values);
  void clearValues();
  size_t valueCount() const;

  // 默认值设置（统一使用 std::optional<std::vector<T>>）
  PositionalArgument<T>& defaultValue(const T& value);  // 单个默认值
  PositionalArgument<T>& defaultValues(const std::vector<T>& values);  // 多个默认值
  bool hasDefaultValues() const;
  const std::vector<T>& getDefaultValues() const;

  // Expected 参数数量控制
  PositionalArgument<T>& expected(int count);  // 精确数量或特殊值（整数）
  PositionalArgument<T>& expected(ExpectedCount count);  // 枚举类型：强类型安全
  PositionalArgument<T>& expected(int min, int max);  // 范围控制
  int getExpectedMin() const;
  int getExpectedMax() const;

  // 外部变量绑定
  PositionalArgument<T>& target(std::vector<T>& external_vector);

  // 验证器（只保留整体验证）
  PositionalArgument<T>& validator(std::function<bool(const std::vector<T>&)> validator);
  bool isValid() const;

  // 链式调用
  PositionalArgument<T>& description(const std::string& description);
  PositionalArgument<T>& required();
  PositionalArgument<T>& callback(std::function<void()> callback);

private:
  std::vector<T> m_values;  // 已解析的值（统一存储）
  std::optional<std::vector<T>> m_default_values;  // 是否存在默认值
  std::vector<T>* m_vector_target;  // 外部变量绑定
  
  int m_expected_min;  // 最小期望数量
  int m_expected_max;  // 最大期望数量
  
  std::function<bool(const std::vector<T>&)> m_validator;  // 整体验证器
};

#include "positional_argument.inc"