#pragma once
#include "option_argument.h"
#include "flag_argument.h"
#include "positional_argument.h"
#include "argument.h"
#include <vector>

class ArgumentParser {
 public:
  ArgumentParser(const std::string& program_name);
  void parse(int argc, char** argv);
  void parse(const std::vector<std::string>& args);
  void printHelp() const;
  
  void addArgument(std::unique_ptr<Argument> argument);
  FlagArgument& addFlagArgument(const std::vector<std::string>& names, 
                                 const std::string& description = "",
                                 bool* target = nullptr);
  
  // 模板成员函数声明
  template <typename T>
  OptionArgument<T>& addOptionArgument(const std::vector<std::string>& names, 
                                        const std::string& description = "",
                                        T* target = nullptr);

  template <typename T>
  PositionalArgument<T>& addPositionalArgument(const std::vector<std::string>& names, 
                                                const std::string& description = "",
                                                std::vector<T>* target = nullptr);

  template <typename T>
  PositionalArgument<T>& addPositionalArgument(const std::string& name, 
                                                const std::string& description = "",
                                                std::vector<T>* target = nullptr);

 private:
  std::string m_program_name;
  std::string m_description;
  std::vector<std::unique_ptr<Argument>> m_args;
  std::vector<std::string> m_positional_args;
};

// 包含模板函数实现
#include "argument_parser.inc"