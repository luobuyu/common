#pragma once
// ArgumentParser 模板成员函数实现
// 此文件被 argument_parser.h 包含
#include "argument_parser.h"
#include "option_argument.h"
#include "positional_argument.h"

// OptionArgument: 不绑定
template <typename T>
OptionArgument<T>& ArgumentParser::addOptionArgument(
    const std::vector<std::string>& names, const std::string& description,
    bool required, std::function<void()> callback) {
  // TODO: 在此处插入 return 语句
}

// OptionArgument: 绑定单个值
template <typename T>
OptionArgument<T>& ArgumentParser::addOptionArgument(
    const std::vector<std::string>& names, T& target,
    const std::string& description, bool required,
    std::function<void()> callback) {
  // 1. 检查名称列表不能为空（严重错误）
  if (names.empty()) {
    throw std::invalid_argument("Argument names cannot be empty");
  }
  
  // 2. 检查每个名称的格式
  for (const auto& name : names) {
    if (name.empty()) {
      throw std::invalid_argument("Argument name cannot be empty string");
    }
    // 选项必须以 - 开头
    if (name[0] != '-') {
      throw std::invalid_argument("Option name must start with '-': " + name);
    }
  }
  
  // 3. 检查名称是否已被使用
  for (const auto& name : names) {
    // 检查是否与已有参数冲突
    for (const auto& arg : m_args) {
      const auto& existing_names = arg->getNames();
      if (std::find(existing_names.begin(), existing_names.end(), name) != existing_names.end()) {
        throw std::invalid_argument("Argument name already exists: " + name);
      }
    }
    // 检查是否与子命令冲突
    if (m_subcommands.count(name)) {
      throw std::invalid_argument("Argument name conflicts with subcommand: " + name);
    }
  }
  
  auto arg = std::make_unique<OptionArgument<T>>(names, description, target, required, callback);
  OptionArgument<T>& ref = *arg;
  addArgument(std::move(arg));
  return ref;
}

// OptionArgument: 绑定 vector
template <typename T>
OptionArgument<T>& ArgumentParser::addOptionArgument(
    const std::vector<std::string>& names, std::vector<T>& target,
    const std::string& description, bool required,
    std::function<void()> callback) {
  // TODO: 在此处插入 return 语句
  auto arg = std::make_unique<OptionArgument<T>>(names, description, target, required, callback);
  OptionArgument<T>& ref = *arg;
  addArgument(std::move(arg));
  return ref;
}

template <typename T>
PositionalArgument<T>& ArgumentParser::addPositionalArgument(
    const std::vector<std::string>& names, const std::string& description,
    bool required, std::function<void()> callback) {
  // TODO: 在此处插入 return 语句
}

template <typename T>
PositionalArgument<T>& ArgumentParser::addPositionalArgument(
    const std::vector<std::string>& names, T& target,
    const std::string& description, bool required,
    std::function<void()> callback) {
  return addPositionalArgument<T>(std::vector<std::string>{names}, description,
                                  required, callback);
}

template <typename T>
PositionalArgument<T>& ArgumentParser::addPositionalArgument(
    const std::vector<std::string>& names, std::vector<T>& target,
    const std::string& description, bool required,
    std::function<void()> callback) {
  // 1. 检查名称列表不能为空（严重错误）
  if (names.empty()) {
    throw std::invalid_argument("Argument names cannot be empty");
  }
  
  // 2. 检查每个名称的格式
  for (const auto& name : names) {
    if (name.empty()) {
      throw std::invalid_argument("Argument name cannot be empty string");
    }
    // 位置参数不能以 - 开头
    if (name[0] == '-') {
      throw std::invalid_argument("Positional argument name cannot start with '-': " + name);
    }
  }
  
  // 3. 检查名称是否已被使用
  for (const auto& name : names) {
    // 检查是否与已有参数冲突
    for (const auto& arg : m_args) {
      const auto& existing_names = arg->getNames();
      if (std::find(existing_names.begin(), existing_names.end(), name) != existing_names.end()) {
        throw std::invalid_argument("Argument name already exists: " + name);
      }
    }
    // 检查是否与子命令冲突
    if (m_subcommands.count(name)) {
      throw std::invalid_argument("Argument name conflicts with subcommand: " + name);
    }
  }
  
  auto arg =
      std::make_unique<PositionalArgument<T>>(names, description, target, required, callback);
  PositionalArgument<T>& ref = *arg;
  addArgument(std::move(arg));
  return ref;
}

