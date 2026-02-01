#include "argument_parser.h"

#include <algorithm>

ArgumentParser::ArgumentParser(const std::string& program_name,
                               const std::string& description,
                               bool add_help)
    : m_program_name(program_name), m_description(description), m_add_help(add_help) {}

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
  // 检查名称冲突
  checkNameConflicts(names);
  // 创建参数对象（使用链式调用设置 required 和 callback）
  auto arg = std::make_unique<FlagArgument>(names, target, description);
  if (required) {
    arg->required();
  }
  if (callback) {
    arg->callback(callback);
  }
  FlagArgument& ref = *arg;
  addArgument(std::move(arg));
  return ref;
}

FlagArgument& ArgumentParser::addFlagArgument(
    const std::vector<std::string>& names, const std::string& description,
    bool required, std::function<void()> callback) {
  // 检查名称冲突
  checkNameConflicts(names);
  // 创建参数对象（使用链式调用设置 required 和 callback）
  auto arg = std::make_unique<FlagArgument>(names, description);
  if (required) {
    arg->required();
  }
  if (callback) {
    arg->callback(callback);
  }
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

bool ArgumentParser::parse(int argc, char** argv) {
  return parse(std::vector<std::string>{argv + 1, argv + argc});
}

bool ArgumentParser::parse(const std::vector<std::string>& args) {
  // 预扫描：检查是否有 --help 或 -h
  // 只有当启用自动 help 且用户没有自定义 -h/--help 时才触发
  if (m_add_help && !hasArgument("-h") && !hasArgument("--help")) {
    for (const auto& arg : args) {
      if (arg == "-h" || arg == "--help") {
        printHelp();
        return false;  // 表示遇到 help 请求，调用者自行决定是否退出
      }
    }
  }

  // 需要先检查是否是子命令
  if (!args.empty()) {
    auto it = m_subcommands.find(args[0]);
    if (it != m_subcommands.end()) {
      return it->second->parse(
          std::vector<std::string>(args.begin() + 1, args.end()));
    }
  }
  
  // 遍历命令行参数，依次匹配并解析
  for (size_t i = 0; i < args.size();) {
    const std::string& arg = args[i];
    
    // 跳过单独出现的 "--"（参数终止符）
    // 当 "--" 没有被某个 Argument 消费时，它会单独出现在主循环中
    if (arg == "--") {
      ++i;
      continue;
    }
    
    bool matched = false;
    // 遍历所有已注册的参数，查找能匹配的参数对象
    Argument* first_not_parsed_pos = nullptr;
    for (const auto& argument : m_args) {
      // 位置参数单独贪婪匹配
      if(argument->getType() == ArgumentType::Positional) {
        if(!argument->isParsed() && first_not_parsed_pos == nullptr) {
          first_not_parsed_pos = argument.get();
        }
        continue;
      }
      if (argument->matches(arg)) {
        // 调用子类的多态 parse 方法，返回值是消耗的参数总数
        size_t consumed = argument->parse(args, i);
        i += consumed;  // 直接跳过消耗的参数数量
        // 找到匹配的参数，标记为已解析
        argument->setParsed(true);
        matched = true;
        break;  // 找到匹配后停止遍历
      }
    }
    if (!matched) {
      if (first_not_parsed_pos != nullptr) {
        size_t consumed = first_not_parsed_pos->parse(args, i);
        i += consumed;  // 位置参数直接跳过消耗的参数数量
        // 找到匹配的参数，标记为已解析
        first_not_parsed_pos->setParsed(true);
      } else {
        throw std::runtime_error("Unknown argument: " + arg);
      }
    }
  }

  // 为所有未被解析的参数应用默认值（syncDefaultValue 内部会检查是否有默认值）
  for (const auto& argument : m_args) {
    if (!argument->isParsed() && argument->hasDefaultValue()) {
      argument->syncDefaultValue();
    }
  }

  // 检查必需参数是否都已解析
  for (const auto& argument : m_args) {
    if (argument->isRequired() && !argument->isParsed()) {
      throw std::runtime_error("Required argument missing: " + 
                               argument->getNames().front());
    }
  }

  return true;  // 正常解析完成
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

bool ArgumentParser::hasArgument(const std::string& name) const {
  for (const auto& arg : m_args) {
    const auto& names = arg->getNames();
    if (std::find(names.begin(), names.end(), name) != names.end()) {
      return true;
    }
  }
  return false;
}

void ArgumentParser::checkNameConflicts(const std::vector<std::string>& names) const {
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
}

void ArgumentParser::printHelp() const {
  std::cout << "Usage: " << m_program_name << " [options]";
  
  // 显示所有位置参数
  for (const auto& arg : m_args) {
    if (arg->getType() == ArgumentType::Positional) {
      std::string name = arg->getNames().front();
      if (arg->isRequired()) {
        std::cout << " <" << name << ">";
      } else {
        std::cout << " [" << name << "]";
      }
    }
  }
  
  // 显示子命令
  if (!m_subcommands.empty()) {
    std::cout << " {";
    bool first = true;
    for (const auto& [name, _] : m_subcommands) {
      if (!first) std::cout << "|";
      std::cout << name;
      first = false;
    }
    std::cout << "}";
  }
  
  std::cout << "\n";
  
  // 显示程序描述
  if (!m_description.empty()) {
    std::cout << "\n" << m_description << "\n";
  }
  
  // 显示位置参数
  bool has_positional = false;
  for (const auto& arg : m_args) {
    if (arg->getType() == ArgumentType::Positional) {
      if (!has_positional) {
        std::cout << "\nPositional arguments:\n";
        has_positional = true;
      }
      std::cout << "  " << arg->getNames().front();
      std::string desc = arg->getDescription();
      if (!desc.empty()) {
        std::cout << "\t" << desc;
      }
      // 显示默认值
      if (arg->hasDefaultValue()) {
        std::cout << " (default: <set>)";
      }
      // 显示是否必需
      if (arg->isRequired()) {
        std::cout << " [required]";
      }
      std::cout << "\n";
    }
  }
  
  // 显示选项参数
  std::cout << "\nOptions:\n";
  
  // 如果启用了自动 help 且用户没有自定义，先打印 -h, --help
  if (m_add_help && !hasArgument("-h") && !hasArgument("--help")) {
    std::cout << "  -h, --help\tshow this help message and exit\n";
  }
  
  for (const auto& arg : m_args) {
    // 跳过位置参数（已经在上面显示了）
    if (arg->getType() == ArgumentType::Positional) {
      continue;
    }
    
    std::cout << "  ";
    const auto& names = arg->getNames();
    for (size_t i = 0; i < names.size(); ++i) {
      if (i > 0) std::cout << ", ";
      std::cout << names[i];
    }
    
    // 显示描述
    std::string desc = arg->getDescription();
    if (!desc.empty()) {
      std::cout << "\t" << desc;
    }
    
    // 显示默认值
    if (arg->hasDefaultValue()) {
      std::cout << " (default: <set>)";
    }
    
    // 显示是否必需
    if (arg->isRequired()) {
      std::cout << " [required]";
    }
    
    std::cout << "\n";
  }
  
  // 显示子命令
  if (!m_subcommands.empty()) {
    std::cout << "\nSubcommands:\n";
    for (const auto& [name, subcmd] : m_subcommands) {
      std::cout << "  " << name;
      // 子命令的描述在其 m_description 中
      std::cout << "\n";
    }
  }
}
