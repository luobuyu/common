#include "argument.h"

namespace dry {
namespace argparse {

// 构造函数（不带 required 和 callback 参数，避免隐式转换问题）
Argument::Argument(const std::vector<std::string>& names,
                   const std::string& Description)
    : m_names{names},
      m_description{Description},
      m_required{false},
      m_parsed{false},
      m_callback{nullptr} {}
Argument::Argument(const ArgumentType& type,
                   const std::vector<std::string>& names,
                   const std::string& Description)
    : m_type{type},
      m_names{names},
      m_description{Description},
      m_required{false},
      m_parsed{false},
      m_callback{nullptr} {}

void Argument::SetDescription(const std::string& Description) {
  m_description = Description;
}

void Argument::SetRequired(bool Required) { m_required = Required; }

void Argument::SetParsed(bool parsed) { m_parsed = parsed; }

void Argument::SetCallback(std::function<void()> Callback) {
  m_callback = Callback;
}

Argument& Argument::Description(const std::string& Description) {
  SetDescription(Description);
  return *this;
}

Argument& Argument::Required() {
  SetRequired(true);
  return *this;
}

Argument& Argument::Callback(std::function<void()> Callback) {
  SetCallback(Callback);
  return *this;
}

const std::vector<std::string>& Argument::GetNames() const { return m_names; }

const std::string& Argument::GetDescription() const { return m_description; }

ArgumentType Argument::GetType() const { return m_type; }

bool Argument::IsRequired() const { return m_required; }

bool Argument::IsParsed() const { return m_parsed; }

}  // namespace argparse
}  // namespace dry