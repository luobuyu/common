#pragma once

#include <functional>
#include <string>
#include <vector>

class Argument {
public:
  // 使用默认参数的构造函数
  Argument(const std::vector<std::string>& names,
           const std::string& description, bool required = false,
           std::function<void()> callback = nullptr);
  virtual ~Argument() = default;

  const std::vector<std::string>& getNames() const;
  const std::string& getDescription() const;
  bool isRequired() const;
  bool isParsed() const;

protected:
  std::vector<std::string> m_names;  // 存储命令行参数的名称以及别名 --help, -h
  std::string m_description;         // 参数的描述信息
  bool m_required;                   // 参数是否必填
  bool m_parsed;                     // 是否已被解析
  std::function<void()> m_callback;  // 参数被成功解析时，会自动调用这个函数
};
