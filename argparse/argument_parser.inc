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
  auto arg = std::make_unique<OptionArgument<T>>(names, description, required, callback);
  
  // 让子类验证自己的名称规则
  arg->validateNames();
  
  // 检查名称冲突
  for (const auto& name : names) {
    for (const auto& existing_arg : m_args) {
      const auto& existing_names = existing_arg->getNames();
      if (std::find(existing_names.begin(), existing_names.end(), name) != existing_names.end()) {
        throw std::invalid_argument("Argument name already exists: " + name);
      }
    }
    if (m_subcommands.count(name)) {
      throw std::invalid_argument("Argument name conflicts with subcommand: " + name);
    }
  }
  
  OptionArgument<T>& ref = *arg;
  addArgument(std::move(arg));
  return ref;
}

// OptionArgument: 绑定单个值
template <typename T>
OptionArgument<T>& ArgumentParser::addOptionArgument(
    const std::vector<std::string>& names, T& target,
    const std::string& description, bool required,
    std::function<void()> callback) {
  auto arg = std::make_unique<OptionArgument<T>>(names, target, description, required, callback);
  
  // 让子类验证自己的名称规则
  arg->validateNames();
  
  // 检查名称冲突
  for (const auto& name : names) {
    for (const auto& existing_arg : m_args) {
      const auto& existing_names = existing_arg->getNames();
      if (std::find(existing_names.begin(), existing_names.end(), name) != existing_names.end()) {
        throw std::invalid_argument("Argument name already exists: " + name);
      }
    }
    if (m_subcommands.count(name)) {
      throw std::invalid_argument("Argument name conflicts with subcommand: " + name);
    }
  }
  
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
  auto arg = std::make_unique<OptionArgument<T>>(names, target, description, required, callback);
  
  // 让子类验证自己的名称规则
  arg->validateNames();
  
  // 检查名称冲突
  for (const auto& name : names) {
    for (const auto& existing_arg : m_args) {
      const auto& existing_names = existing_arg->getNames();
      if (std::find(existing_names.begin(), existing_names.end(), name) != existing_names.end()) {
        throw std::invalid_argument("Argument name already exists: " + name);
      }
    }
    if (m_subcommands.count(name)) {
      throw std::invalid_argument("Argument name conflicts with subcommand: " + name);
    }
  }
  
  OptionArgument<T>& ref = *arg;
  addArgument(std::move(arg));
  return ref;
}

template <typename T>
PositionalArgument<T>& ArgumentParser::addPositionalArgument(
    const std::vector<std::string>& names, const std::string& description,
    bool required, std::function<void()> callback) {
  auto arg = std::make_unique<PositionalArgument<T>>(names, description, required, callback);
  
  // 让子类验证自己的名称规则
  arg->validateNames();
  
  // 检查名称冲突
  for (const auto& name : names) {
    for (const auto& existing_arg : m_args) {
      const auto& existing_names = existing_arg->getNames();
      if (std::find(existing_names.begin(), existing_names.end(), name) != existing_names.end()) {
        throw std::invalid_argument("Argument name already exists: " + name);
      }
    }
    if (m_subcommands.count(name)) {
      throw std::invalid_argument("Argument name conflicts with subcommand: " + name);
    }
  }
  
  PositionalArgument<T>& ref = *arg;
  addArgument(std::move(arg));
  return ref;
}

template <typename T>
PositionalArgument<T>& ArgumentParser::addPositionalArgument(
    const std::vector<std::string>& names, T& target,
    const std::string& description, bool required,
    std::function<void()> callback) {
  auto arg = std::make_unique<PositionalArgument<T>>(names, target, description, required, callback);
  
  // 让子类验证自己的名称规则
  arg->validateNames();
  
  // 检查名称冲突
  for (const auto& name : names) {
    for (const auto& existing_arg : m_args) {
      const auto& existing_names = existing_arg->getNames();
      if (std::find(existing_names.begin(), existing_names.end(), name) != existing_names.end()) {
        throw std::invalid_argument("Argument name already exists: " + name);
      }
    }
    if (m_subcommands.count(name)) {
      throw std::invalid_argument("Argument name conflicts with subcommand: " + name);
    }
  }
  
  PositionalArgument<T>& ref = *arg;
  addArgument(std::move(arg));
  return ref;
}

template <typename T>
PositionalArgument<T>& ArgumentParser::addPositionalArgument(
    const std::vector<std::string>& names, std::vector<T>& target,
    const std::string& description, bool required,
    std::function<void()> callback) {
  auto arg = std::make_unique<PositionalArgument<T>>(names, target, description, required, callback);
  
  // 让子类验证自己的名称规则
  arg->validateNames();
  
  // 检查名称冲突
  for (const auto& name : names) {
    for (const auto& existing_arg : m_args) {
      const auto& existing_names = existing_arg->getNames();
      if (std::find(existing_names.begin(), existing_names.end(), name) != existing_names.end()) {
        throw std::invalid_argument("Argument name already exists: " + name);
      }
    }
    if (m_subcommands.count(name)) {
      throw std::invalid_argument("Argument name conflicts with subcommand: " + name);
    }
  }
  
  PositionalArgument<T>& ref = *arg;
  addArgument(std::move(arg));
  return ref;
}

