#pragma once
// ArgumentParser 模板成员函数实现
// 此文件被 argument_parser.h 包含

#include "option_argument.h"
#include "positional_argument.h"

template <typename T>
OptionArgument<T>& ArgumentParser::addOptionArgument(
    const std::vector<std::string>& names,
    const std::string& description,
    T* target) {
  auto arg = std::make_unique<OptionArgument<T>>(names, description, target);
  OptionArgument<T>& ref = *arg;
  addArgument(std::move(arg));
  return ref;
}

template <typename T>
PositionalArgument<T>& ArgumentParser::addPositionalArgument(
    const std::vector<std::string>& names,
    const std::string& description,
    std::vector<T>* target) {
  auto arg = std::make_unique<PositionalArgument<T>>(names, description, target);
  PositionalArgument<T>& ref = *arg;
  addArgument(std::move(arg));
  return ref;
}

template <typename T>
PositionalArgument<T>& ArgumentParser::addPositionalArgument(
    const std::string& name,
    const std::string& description,
    std::vector<T>* target) {
  return addPositionalArgument<T>(std::vector<std::string>{name}, description, target);
}
