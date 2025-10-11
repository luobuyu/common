#pragma once
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>

#include "argument.h"
#include "flag_argument.h"
#include "option_argument.h"
#include "positional_argument.h"


/**
 * 使用时需要先创建 ArgumentParser 对象
 * 然后通过 addFlagArgument、addOptionArgument、addPositionalArgument 方法添加参数  
 * 最后调用 parse 方法解析命令行参数
 */
class ArgumentParser {
 public:
  ArgumentParser(const std::string& program_name,
                 const std::string& description = "");
  // 解析命令行参数
  // 抛出异常：std::runtime_error (解析错误)
  void parse(int argc, char** argv);
  void parse(const std::vector<std::string>& args);
  void printHelp() const;
  Argument& addArgument(const std::vector<std::string>& names,
                        const std::string& description = "",
                        bool required = false,
                        std::function<void()> callback = nullptr);

  // 添加参数的高级接口
  FlagArgument& addFlagArgument(const std::vector<std::string>& names,
                                const std::string& description = "",
                                bool* target = nullptr, bool required = false,
                                std::function<void()> callback = nullptr);

  // 模板成员函数声明
  template <typename T>
  OptionArgument<T>& addOptionArgument(
      const std::vector<std::string>& names,
      const std::string& description = "", T* target = nullptr,
      bool required = false, std::function<void()> callback = nullptr);

  template <typename T>
  PositionalArgument<T>& addPositionalArgument(
      const std::vector<std::string>& names,
      const std::string& description = "", std::vector<T>* target = nullptr,
      bool required = false, std::function<void()> callback = nullptr);

  template <typename T>
  PositionalArgument<T>& addPositionalArgument(
      const std::string& name, const std::string& description = "",
      std::vector<T>* target = nullptr, bool required = false,
      std::function<void()> callback = nullptr);

  // 子命令支持
  ArgumentParser& addSubcommand(const std::string& name,
                                const std::string& description = "");

 private:
  // 内部辅助方法：添加已创建的参数对象
  Argument& addArgument(std::unique_ptr<Argument> argument);

  std::string m_program_name;  // 程序名称
  std::string m_description;   // 程序描述
  std::vector<std::unique_ptr<Argument>> m_args;  // 选项参数、开关参数、位置参数
  std::unordered_map<std::string, std::unique_ptr<ArgumentParser>> m_subcommands;  // 子命令 [command name -> ArgumentParser]
};

// 包含模板函数实现
#include "argument_parser.inc"