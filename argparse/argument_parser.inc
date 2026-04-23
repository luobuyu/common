#pragma once
// ArgumentParser 模板成员函数实现
// 此文件被 argument_parser.h 包含
#include "argument_parser.h"
#include "option_argument.h"
#include "positional_argument.h"

namespace dry {
namespace argparse {

// OptionArgument: 不绑定
template <typename T>
OptionArgument<T>& ArgumentParser::addOptionArgument(
    const std::vector<std::string>& names, const std::string& description) {
  auto arg = std::make_unique<OptionArgument<T>>(names, description);

  // 让子类验证自己的名称规则
  arg->validateNames();

  // 检查名称冲突
  checkNameConflicts(names);

  OptionArgument<T>& ref = *arg;
  addArgument(std::move(arg));
  return ref;
}

// OptionArgument: 绑定单个值
template <typename T>
OptionArgument<T>& ArgumentParser::addOptionArgument(
    const std::vector<std::string>& names, T& target,
    const std::string& description) {
  auto arg = std::make_unique<OptionArgument<T>>(names, target, description);

  // 让子类验证自己的名称规则
  arg->validateNames();

  // 检查名称冲突
  checkNameConflicts(names);

  OptionArgument<T>& ref = *arg;
  addArgument(std::move(arg));
  return ref;
}

// OptionArgument: 绑定 vector
template <typename T>
OptionArgument<T>& ArgumentParser::addOptionArgument(
    const std::vector<std::string>& names, std::vector<T>& target,
    const std::string& description) {
  auto arg = std::make_unique<OptionArgument<T>>(names, target, description);

  // 让子类验证自己的名称规则
  arg->validateNames();

  // 检查名称冲突
  checkNameConflicts(names);

  OptionArgument<T>& ref = *arg;
  addArgument(std::move(arg));
  return ref;
}

template <typename T>
PositionalArgument<T>& ArgumentParser::addPositionalArgument(
    const std::vector<std::string>& names, const std::string& description) {
  auto arg = std::make_unique<PositionalArgument<T>>(names, description);

  // 让子类验证自己的名称规则
  arg->validateNames();

  // 检查名称冲突
  checkNameConflicts(names);

  PositionalArgument<T>& ref = *arg;
  addArgument(std::move(arg));
  return ref;
}

template <typename T>
PositionalArgument<T>& ArgumentParser::addPositionalArgument(
    const std::vector<std::string>& names, T& target,
    const std::string& description) {
  auto arg =
      std::make_unique<PositionalArgument<T>>(names, target, description);

  // 让子类验证自己的名称规则
  arg->validateNames();

  // 检查名称冲突
  checkNameConflicts(names);

  PositionalArgument<T>& ref = *arg;
  addArgument(std::move(arg));
  return ref;
}

template <typename T>
PositionalArgument<T>& ArgumentParser::addPositionalArgument(
    const std::vector<std::string>& names, std::vector<T>& target,
    const std::string& description) {
  auto arg =
      std::make_unique<PositionalArgument<T>>(names, target, description);

  // 让子类验证自己的名称规则
  arg->validateNames();

  // 检查名称冲突
  checkNameConflicts(names);

  PositionalArgument<T>& ref = *arg;
  addArgument(std::move(arg));
  return ref;
}

}  // namespace argparse
}  // namespace dry
