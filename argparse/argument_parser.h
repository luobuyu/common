#pragma once
#include "option_argument.h"
#include <vector>

class ArgumentParser {
 public:
  ArgumentParser(const std::string& program_name);
  void addArgument(Argument* argument);
  void parse(int argc, char** argv);
  void printHelp() const;

 private:
  std::string m_program_name;
  std::vector<Argument*> m_args;
  std::vector<std::string> m_positional_args;
};