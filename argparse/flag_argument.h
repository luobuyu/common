#pragma once

#include "argument.h"

class FlagArgument : public Argument {
 public:
  // 使用默认参数的构造函数
  FlagArgument(const std::vector<std::string>& names,
               const std::string& description, bool* target = nullptr,
               bool required = false, std::function<void()> callback = nullptr);
  FlagArgument(const std::vector<std::string>& names,
               const std::string& description, bool required = false,
               std::function<void()> callback = nullptr);

  bool getFlag() const;
  void setFlag(bool value);

 private:
  bool m_flag;     // 存储标志参数的状态，true表示启用，false表示禁用
  bool* m_target;  // 指向一个bool变量，用于存储标志参数的状态
};
