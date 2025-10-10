#include "argument_parser.h"


ArgumentParser::ArgumentParser(const std::string& program_name)
    : m_program_name(program_name) {}

void ArgumentParser::addArgument(std::unique_ptr<Argument> argument) {
  m_args.push_back(std::move(argument));
} 

FlagArgument& ArgumentParser::addFlagArgument(
    const std::vector<std::string>& names, 
    const std::string& description,
    bool* target) {
  auto arg = std::make_unique<FlagArgument>(names, description, target);
  FlagArgument& ref = *arg;
  addArgument(std::move(arg));
  return ref;
}

void ArgumentParser::parse(int argc, char** argv) {
  std::vector<std::string> args;
  for (int i = 0; i < argc; ++i) {
    args.push_back(argv[i]);
  }
  parse(args);
}

void ArgumentParser::parse(const std::vector<std::string>& args) {
  // 简单的解析逻辑示例
  for (size_t i = 1; i < args.size(); ++i) { // 跳过程序名
    const std::string& arg = args[i];
    bool matched = false;
    for (const auto& argument : m_args) {
      const auto& names = argument->getNames();
      if (std::find(names.begin(), names.end(), arg) != names.end()) {
        argument->setParsed(true);
        matched = true;
        // 如果是 FlagArgument，设置为 true
        if (auto flag_arg = dynamic_cast<FlagArgument*>(argument.get())) {
          flag_arg->setFlag(true);
        }
        // 如果是 OptionArgument，读取下一个值
        else if (auto option_arg = dynamic_cast<OptionArgument<std::string>*>(argument.get())) {
          if (i + 1 < args.size()) {
            option_arg->setValue(args[++i]);
          } else {
            throw std::runtime_error("Expected value after " + arg);
          }
        }
        // 如果是 PositionalArgument，添加值
        else if (auto pos_arg = dynamic_cast<PositionalArgument<std::string>*>(argument.get())) {
          pos_arg->addValue(arg);
        }
        break;
      }
    }
    if (!matched) {
      m_positional_args.push_back(arg); // 未匹配的参数视为位置参数
    }
  }

  // 检查必需参数是否都已解析
  // 策略: 遵循 Python argparse 风格
  // - required=true: 必须从命令行提供，忽略默认值
  // - required=false: 可以不提供，使用默认值
  for (const auto& argument : m_args) {
    if (argument->isRequired() && !argument->isParsed()) {
      throw std::runtime_error("Required argument missing: " + argument->getNames().front());
    }
  }
}

void ArgumentParser::printHelp() const {
  std::cout << "Usage: " << m_program_name << " [options] ";
  for (const auto& arg : m_args) {
    if (dynamic_cast<PositionalArgument<std::string>*>(arg.get())) {
      std::cout << arg->getNames().front() << " ";
    }
  }
  std::cout << "\n\nOptions:\n";
  for (const auto& arg : m_args) {
    std::string desc = arg->getDescription();
    if (desc.empty()) {
      desc = "(no description)";  // 或者显示为灰色、斜体等
    }
    std::cout << "  ";
    for (const auto& name : arg->getNames()) {
      std::cout << name << ", ";
    }
    std::cout << "\b\b\t" << desc << "\n";
  }
}