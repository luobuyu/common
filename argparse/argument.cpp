#include "argument.h"

// 使用默认参数的构造函数
Argument::Argument(const std::vector<std::string>& names, 
                   const std::string& description,
                   bool required,
                   std::function<void()> callback)
  : m_names{names}, m_description{description}, m_required{required}, 
    m_parsed{false}, m_callback{callback} {
}

const std::vector<std::string>& Argument::getNames() const {
  return m_names;
}

const std::string& Argument::getDescription() const {
  return m_description;
}

bool Argument::isRequired() const {
  return m_required;
}

bool Argument::isParsed() const {
  return m_parsed;
}
