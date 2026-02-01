#pragma once

#include <functional>
#include <string>
#include <vector>

// 参数类型枚举
enum class ArgumentType {
  Flag,       // 标志参数（如 -v, --verbose）
  Option,     // 选项参数（如 -o file, --output file）
  Positional  // 位置参数（如 input.txt）
};

class Argument {
public:
  // 构造函数（不带 required 和 callback 参数，避免隐式转换问题）
  // 使用链式调用 .required() 和 .callback() 来设置
  Argument(const std::vector<std::string>& names,
           const std::string& description);
  Argument(const ArgumentType& type, const std::vector<std::string>& names,
           const std::string& description);
  virtual ~Argument() = default;

  // setter
  void setDescription(const std::string& description);
  void setRequired(bool required);
  void setParsed(bool parsed);
  void setCallback(std::function<void()> callback);

  // 链式调用
  Argument& description(const std::string& description);
  Argument& required();
  Argument& callback(std::function<void()> callback);

  const std::vector<std::string>& getNames() const;
  const std::string& getDescription() const;
  bool isRequired() const;
  bool isParsed() const;
  ArgumentType getType() const;

  // 验证参数名称是否合法（由子类实现具体规则）
  virtual void validateNames() const = 0;

  // 不同子类的实现:
  //   - FlagArgument/OptionArgument: 检查 arg 以 '-' 开头，是否在 m_names 中（精确匹配）
  //   - PositionalArgument: 检查 arg 是否不以 '-' 开头（任何非选项都可能是位置参数）
  virtual bool matches(const std::string& arg) const = 0;
  
  // 解析命令行参数
  // 参数:
  //   args: 完整的命令行参数列表
  //   current_index: 当前匹配到的参数位置（对于选项参数，是 --name 的位置）
  // 返回值: 消耗的参数总数量（包括 current_index 本身）
  //   - FlagArgument: 返回 1（消耗标志参数本身）
  //   - OptionArgument: 返回 1 + 消耗的值的个数
  //   - PositionalArgument: 返回实际消耗的参数数量
  virtual size_t parse(const std::vector<std::string>& args, size_t current_index) = 0;

  // 检查是否设置了默认值（子类应重写此方法）
  virtual bool hasDefaultValue() const { return false; }

  // 将默认值同步到绑定的外部变量（对于未被解析的参数）
  // 内部会检查 hasDefaultValue，只有设置了默认值时才执行同步
  // 默认实现调用 m_sync_to_target（如果有的话）
  virtual void syncDefaultValue() {
    if (hasDefaultValue() && m_sync_to_target) {
      m_sync_to_target();
    }
  }

  // ========== 验证器接口 ==========
  // 检查参数值是否有效（由子类实现具体验证逻辑）
  // 返回 true 表示验证通过，false 表示验证失败
  virtual bool isValid() const = 0;

  // 验证参数值，如果无效则抛出异常
  // 默认实现：调用 isValid()，失败时抛出 std::invalid_argument
  virtual void validate() const = 0;

 protected:
  ArgumentType m_type;               // 参数类型
  std::vector<std::string> m_names;  // 存储命令行参数的名称以及别名 --help, -h
  std::string m_description;         // 参数的描述信息
  bool m_required;                   // 参数是否必填
  bool m_parsed;                     // 是否已被解析
  std::function<void()> m_callback;  // 参数被成功解析时，会自动调用这个函数
  std::function<void()> m_sync_to_target; // 用于将解析出的变量同步到绑定的外部变量
};
