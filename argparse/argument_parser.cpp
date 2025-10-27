#include "argument_parser.h"

ArgumentParser::ArgumentParser(const std::string& program_name,
                               const std::string& description)
    : m_program_name(program_name), m_description(description) {}

Argument& ArgumentParser::addArgument(std::unique_ptr<Argument> argument) {
  Argument& ref = *argument;
  validateNames(ref);
  m_args.emplace_back(std::move(argument));
  return ref;
}

FlagArgument& ArgumentParser::addFlagArgument(
    const std::vector<std::string>& names, bool& target,
    const std::string& description, bool required,
    std::function<void()> callback) {
  // 创建参数对象
  auto arg = std::make_unique<FlagArgument>(names, target, description, required, callback);
  FlagArgument& ref = *arg;
  addArgument(std::move(arg));
  return ref;
}

FlagArgument& ArgumentParser::addFlagArgument(
    const std::vector<std::string>& names, const std::string& description,
    bool required, std::function<void()> callback) {
  std::unique_ptr<FlagArgument> arg;
  arg = std::make_unique<FlagArgument>(names, description, required, callback);
  FlagArgument& ref = *arg;
  addArgument(std::move(arg));
  return ref;
}

ArgumentParser& ArgumentParser::addSubcommand(const std::string& name,
                                              const std::string& description) {
  // 1. 检查子命令名称不能为空（严重错误）
  if (name.empty()) {
    throw std::invalid_argument("Subcommand name cannot be empty");
  }

  // 2. 子命令名称不能以 - 开头
  if (name[0] == '-') {
    throw std::invalid_argument("Subcommand name cannot start with '-': " + name);
  }

  // 3. 检查子命令名称是否已存在
  if (m_subcommands.count(name)) {
    throw std::invalid_argument("Subcommand already exists: " + name);
  }

  // 4. 检查是否与已有参数名冲突
  for (const auto& arg : m_args) {
    const auto& names = arg->getNames();
    if (std::find(names.begin(), names.end(), name) != names.end()) {
      throw std::invalid_argument("Subcommand name conflicts with argument: " + name);
    }
  }

  auto subcommand = std::make_unique<ArgumentParser>(name, description);
  ArgumentParser& ref = *subcommand;
  m_subcommands[name] = std::move(subcommand);
  return ref;
}

void ArgumentParser::parse(int argc, char** argv) {
  parse(std::vector<std::string>{argv + 1, argv + argc});
}

void ArgumentParser::parse(const std::vector<std::string>& args) {
  // 简单的解析逻辑示例
  // 需要先检查是否是子命令
  if (!args.empty()) {
    auto it = m_subcommands.find(args[0]);
    if (it != m_subcommands.end()) {
      it->second->parse(
          std::vector<std::string>(args.begin() + 1, args.end()));
      return;
    }
  }
  
  // 遍历命令行参数，依次匹配并解析
  for (size_t i = 0; i < args.size();) {
    const std::string& arg = args[i];
    bool matched = false;
    // 遍历所有已注册的参数，查找能匹配的参数对象
    for (const auto& argument : m_args) {
      // 调用多态方法检查是否匹配
      if (argument->matches(arg)) {
        // 调用子类的多态 parse 方法
        // 返回值表示当前选项消耗的参数个数
        size_t consumed = argument->parse(args, i);
        i += consumed;  // 跳过已消耗的参数
        // 找到匹配的参数，标记为已解析
        argument->setParsed(true);
        matched = true;
        break;  // 找到匹配后停止遍历
      }
    }
    
    if (!matched) {
      // 未匹配到任何参数：抛出异常
      throw std::runtime_error("Unknown or unexpected argument: " + arg);
    }
  }

  // 检查必需参数是否都已解析
  // 策略: 遵循 Python argparse 风格
  // - required=true: 必须从命令行提供，忽略默认值
  // - required=false: 可以不提供，使用默认值
  for (const auto& argument : m_args) {
    if (argument->isRequired() && !argument->isParsed()) {
      throw std::runtime_error("Required argument missing: " + 
                               argument->getNames().front());
    }
  }
}

void ArgumentParser::validateNames(const Argument& argument) const {
  // 遍历所有参数，调用各自的 validateNames 方法
  argument.validateNames();
  // 验证是否出现冲突的名字
  for(const auto& name: argument.getNames()) {
    if (m_subcommands.count(name)) {
      throw std::invalid_argument("Subcommand name conflicts with argument: " + name);
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
