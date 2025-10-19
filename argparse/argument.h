#pragma once

#include <functional>
#include <string>
#include <vector>

class Argument {
public:
  // 使用默认参数的构造函数
  Argument(const std::vector<std::string>& names,
           const std::string& description, bool required = false,
           std::function<void()> callback = nullptr);
  virtual ~Argument() = default;

  // setter
  void setDescription(const std::string& description);
  void setRequired(bool required);
  void setParsed(bool parsed);
  void setCallback(std::function<void()> callback);

  const std::vector<std::string>& getNames() const;
  const std::string& getDescription() const;
  bool isRequired() const;
  bool isParsed() const;

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
  // 返回值: 消耗的额外参数数量（不包括 current_index 本身）
  //   - FlagArgument: 返回 0（不需要额外参数）
  //   - OptionArgument: 返回 1（需要 args[current_index+1] 作为值）
  //   - PositionalArgument: 返回实际消耗的参数数量
  virtual size_t parse(const std::vector<std::string>& args, size_t current_index) = 0;

protected:
  std::vector<std::string> m_names;  // 存储命令行参数的名称以及别名 --help, -h
  std::string m_description;         // 参数的描述信息
  bool m_required;                   // 参数是否必填
  bool m_parsed;                     // 是否已被解析
  std::function<void()> m_callback;  // 参数被成功解析时，会自动调用这个函数
  std::function<void()> m_sync_to_target; // 用于将解析出的变量同步到绑定的外部变量
};
