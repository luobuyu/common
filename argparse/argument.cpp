#include "argument.h"

namespace dry {
namespace argparse {

// 构造函数（不带 required 和 callback 参数，避免隐式转换问题）
Argument::Argument(const std::vector<std::string>& names,
                   const std::string& description)
    : m_names{names},
      m_description{description},
      m_required{false},
      m_parsed{false},
      m_callback{nullptr} {}
Argument::Argument(const ArgumentType& type,
                   const std::vector<std::string>& names,
                   const std::string& description)
    : m_type{type},
      m_names{names},
      m_description{description},
      m_required{false},
      m_parsed{false},
      m_callback{nullptr} {}

void Argument::SetDescription(const std::string& description) {
  m_description = description;
}

void Argument::SetRequired(bool required) { m_required = required; }

void Argument::SetParsed(bool parsed) { m_parsed = parsed; }

void Argument::SetCallback(std::function<void()> callback) {
  m_callback = callback;
}

Argument& Argument::description(const std::string& description) {
  SetDescription(description);
  return *this;
}

Argument& Argument::required() {
  SetRequired(true);
  return *this;
}

Argument& Argument::callback(std::function<void()> callback) {
  SetCallback(callback);
  return *this;
}

const std::vector<std::string>& Argument::GetNames() const { return m_names; }

const std::string& Argument::GetDescription() const { return m_description; }

ArgumentType Argument::GetType() const { return m_type; }

bool Argument::IsRequired() const { return m_required; }

bool Argument::IsParsed() const { return m_parsed; }

}  // namespace argparse
}  // namespace dry