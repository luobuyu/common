#include "argument.h"

// 构造函数（不带 required 和 callback 参数，避免隐式转换问题）
Argument::Argument(const std::vector<std::string>& names,
                   const std::string& description)
  : m_names{names}, m_description{description}, m_required{false}, 
    m_parsed{false}, m_callback{nullptr} {
}
Argument::Argument(const ArgumentType& type,
                   const std::vector<std::string>& names,
                   const std::string& description)
  : m_type{type}, m_names{names}, m_description{description}, m_required{false},
    m_parsed{false}, m_callback{nullptr} {}

void Argument::setDescription(const std::string& description) {
  m_description = description;
}

void Argument::setRequired(bool required) {
  m_required = required;
}

void Argument::setParsed(bool parsed) {
  m_parsed = parsed;
}

void Argument::setCallback(std::function<void()> callback) {
  m_callback = callback;
}

Argument& Argument::description(const std::string& description) {
  setDescription(description);
  return *this;
}

Argument& Argument::required() {
  setRequired(true);
  return *this;
}

Argument& Argument::callback(std::function<void()> callback) {
  setCallback(callback);
  return *this;
}

const std::vector<std::string>& Argument::getNames() const {
  return m_names;
}

const std::string& Argument::getDescription() const {
  return m_description;
}

ArgumentType Argument::getType() const {
  return m_type;
}

bool Argument::isRequired() const {
  return m_required;
}

bool Argument::isParsed() const {
  return m_parsed;
}