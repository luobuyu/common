// 演示类内枚举的多种用法
#include "positional_argument.h"
#include <iostream>
#include <string>

int main() {
  std::cout << "=== PositionalArgument::ExpectedCount 枚举用法示例 ===" << std::endl;
  
  std::cout << "\n【方案1】完整路径 - 最明确" << std::endl;
  {
    std::vector<std::string> files_storage;
    PositionalArgument<std::string> files(
        {"files"}, 
        "Input files",
        &files_storage
    );
    // 完整枚举路径：类型安全，编译时检查
    files.expected(PositionalArgument<std::string>::ExpectedCount::Unlimited);
    std::cout << "  files.expected(PositionalArgument<string>::ExpectedCount::Unlimited)" << std::endl;
    std::cout << "  → min=" << files.getExpectedMin() << ", max=" << files.getExpectedMax() << std::endl;
  }
  
  std::cout << "\n【方案2】类型别名 - 推荐！" << std::endl;
  {
    // 类型别名让代码更简洁
    using PosArg = PositionalArgument<int>;
    using ExpCount = PosArg::ExpectedCount;
    
    std::vector<int> numbers_storage;
    PosArg numbers({"numbers"}, "Numbers", &numbers_storage);
    numbers.expected(ExpCount::AtLeastOne);  // 简洁清晰！
    
    std::cout << "  using ExpCount = PositionalArgument<int>::ExpectedCount;" << std::endl;
    std::cout << "  numbers.expected(ExpCount::AtLeastOne)" << std::endl;
    std::cout << "  → min=" << numbers.getExpectedMin() << ", max=" << numbers.getExpectedMax() << std::endl;
  }
  
  std::cout << "\n【方案3】auto 推导 - 最简洁" << std::endl;
  {
    std::vector<double> values_storage;
    PositionalArgument<double> values({"values"}, "Values", &values_storage);
    
    // 使用 auto 自动推导枚举类型
    auto unlimited = PositionalArgument<double>::ExpectedCount::Unlimited;
    values.expected(unlimited);
    
    std::cout << "  auto unlimited = PositionalArgument<double>::ExpectedCount::Unlimited;" << std::endl;
    std::cout << "  values.expected(unlimited)" << std::endl;
    std::cout << "  → min=" << values.getExpectedMin() << ", max=" << values.getExpectedMax() << std::endl;
  }
  
  std::cout << "\n【方案4】常量版本 - 向后兼容" << std::endl;
  {
    std::vector<std::string> paths_storage;
    PositionalArgument<std::string> paths({"paths"}, "Paths", &paths_storage);
    
    // 仍然可以使用整数常量（向后兼容）
    paths.expected(PositionalArgument<std::string>::EXPECTED_UNLIMITED);
    
    std::cout << "  paths.expected(PositionalArgument<string>::EXPECTED_UNLIMITED)" << std::endl;
    std::cout << "  → min=" << paths.getExpectedMin() << ", max=" << paths.getExpectedMax() << std::endl;
  }
  
  std::cout << "\n【方案5】直接整数 - 精确数量" << std::endl;
  {
    std::vector<int> coords_storage;
    PositionalArgument<int> coords({"x", "y", "z"}, "Coordinates", &coords_storage);
    
    // 精确数量：直接用整数
    coords.expected(3);
    
    std::cout << "  coords.expected(3)  // 精确3个参数" << std::endl;
    std::cout << "  → min=" << coords.getExpectedMin() << ", max=" << coords.getExpectedMax() << std::endl;
  }
  
  std::cout << "\n【方案6】范围控制" << std::endl;
  {
    std::vector<std::string> names_storage;
    PositionalArgument<std::string> names({"names"}, "Names", &names_storage);
    
    // 范围：1到5个
    names.expected(1, 5);
    
    std::cout << "  names.expected(1, 5)  // 1-5个参数" << std::endl;
    std::cout << "  → min=" << names.getExpectedMin() << ", max=" << names.getExpectedMax() << std::endl;
  }
  
  std::cout << "\n=== 类型安全验证 ===" << std::endl;
  {
    using ExpCount = PositionalArgument<int>::ExpectedCount;
    
    // 编译时类型检查
    ExpCount count = ExpCount::Unlimited;
    
    // 可以安全地进行 switch
    switch (count) {
      case ExpCount::Unlimited:
        std::cout << "  ✓ 检测到 Unlimited 模式" << std::endl;
        break;
      case ExpCount::AtLeastOne:
        std::cout << "  ✓ 检测到 AtLeastOne 模式" << std::endl;
        break;
    }
    
    // 整数转枚举（显式转换）
    int value = -1;
    ExpCount count2 = static_cast<ExpCount>(value);
    if (count2 == ExpCount::Unlimited) {
      std::cout << "  ✓ 整数 -1 正确转换为 Unlimited" << std::endl;
    }
  }
  
  std::cout << "\n=== 推荐用法总结 ===" << std::endl;
  std::cout << "  1. 使用类型别名简化：using ExpCount = PositionalArgument<T>::ExpectedCount;" << std::endl;
  std::cout << "  2. 特殊值用枚举：.expected(ExpCount::Unlimited)" << std::endl;
  std::cout << "  3. 精确数量用整数：.expected(3)" << std::endl;
  std::cout << "  4. 范围用两参数：.expected(1, 5)" << std::endl;
  
  return 0;
}
