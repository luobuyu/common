#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "argument.h"
#include "flag_argument.h"
#include "option_argument.h"
#include "positional_argument.h"

namespace dry {
namespace argparse {

/**
 * 使用时需要先创建 ArgumentParser 对象
 * 然后通过 AddFlagArgument、AddOptionArgument、AddPositionalArgument
 * 方法添加参数 最后调用 Parse 方法解析命令行参数
 */
class ArgumentParser {
 public:
  ArgumentParser(const std::string& program_name,
                 const std::string& Description = "", bool add_help = true);
  // 解析命令行参数
  // 返回值：true 表示正常解析完成，false 表示遇到 --help 请求（已打印帮助信息）
  // 抛出异常：std::runtime_error (解析错误)
  bool Parse(int argc, char** argv);
  bool Parse(const std::vector<std::string>& args);
  void PrintHelp() const;

  // 添加参数的高级接口
  // FlagArgument: 支持外部绑定
  FlagArgument& AddFlagArgument(const std::vector<std::string>& names,
                                bool& target,
                                const std::string& Description = "",
                                bool Required = false,
                                std::function<void()> Callback = nullptr);
  // 添加参数的高级接口
  // FlagArgument: 不支持外部绑定
  FlagArgument& AddFlagArgument(const std::vector<std::string>& names,
                                const std::string& Description = "",
                                bool Required = false,
                                std::function<void()> Callback = nullptr);

  // OptionArgument: 不绑定(使用默认参数会有歧义,所以单独声明)
  template <typename T>
  OptionArgument<T>& AddOptionArgument(const std::vector<std::string>& names,
                                       const std::string& Description = "");
  // OptionArgument: 绑定单个值
  template <typename T>
  OptionArgument<T>& AddOptionArgument(const std::vector<std::string>& names,
                                       T& target,
                                       const std::string& Description = "");

  // OptionArgument: 绑定 vector
  template <typename T>
  OptionArgument<T>& AddOptionArgument(const std::vector<std::string>& names,
                                       std::vector<T>& target,
                                       const std::string& Description = "");

  // PositionalArgument: 不绑定
  template <typename T>
  PositionalArgument<T>& AddPositionalArgument(
      const std::vector<std::string>& names,
      const std::string& Description = "");

  // PositionalArgument: 绑定单个值
  template <typename T>
  PositionalArgument<T>& AddPositionalArgument(
      const std::vector<std::string>& names, T& target,
      const std::string& Description = "");

  // PositionalArgument: 绑定 vector
  template <typename T>
  PositionalArgument<T>& AddPositionalArgument(
      const std::vector<std::string>& names, std::vector<T>& target,
      const std::string& Description = "");

  // 子命令支持
  ArgumentParser& AddSubcommand(const std::string& name,
                                const std::string& Description = "");

 private:
  // 内部辅助方法：添加已创建的参数对象
  Argument& AddArgument(std::unique_ptr<Argument> argument);
  void ValidateNames(const Argument& argument) const;
  // 检查是否已定义某个参数名
  bool HasArgument(const std::string& name) const;
  // 检查参数名称是否与已有参数或子命令冲突
  void CheckNameConflicts(const std::vector<std::string>& names) const;

  std::string m_program_name;  // 程序名称
  std::string m_description;   // 程序描述
  bool m_add_help;             // 是否自动添加 --help 支持
  std::vector<std::unique_ptr<Argument>>
      m_args;  // 选项参数、开关参数、位置参数
  std::unordered_map<std::string, std::unique_ptr<ArgumentParser>>
      m_subcommands;  // 子命令 [command name -> ArgumentParser]
};

}  // namespace argparse
}  // namespace dry

// 包含模板函数实现
#include "argument_parser.inc"