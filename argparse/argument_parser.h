#pragma once
#include "option_argument.h"
#include <vector>

class ArgumentParser {
 public:
  ArgumentParser(const std::string& program_name);
  void addArgument(const std::unique_ptr<Argument>& argument);
  void parse(int argc, char** argv);
  void printHelp() const;

 private:
  std::string m_program_name;
  std::string m_description;
  std::vector<std::unique_ptr<Argument>> m_args;
  std::vector<std::string> m_positional_args;
};