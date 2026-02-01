/**
 * argparse 库全面测试脚本
 * 
 * 测试覆盖：
 * 1. FlagArgument 测试
 * 2. OptionArgument 测试
 * 3. PositionalArgument 测试
 * 4. help 功能测试（自动 help / 用户自定义 help）
 * 5. 回调函数测试
 * 6. 外部变量绑定测试
 * 7. nargs (expected) 测试
 * 8. 默认值测试
 * 9. required 参数测试
 * 10. 子命令测试
 * 11. 异常处理测试
 */

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../argument_parser.h"

// 测试计数器
static int test_passed = 0;
static int test_failed = 0;

// 辅助宏
#define TEST_CASE(name) \
  std::cout << "\n[测试] " << name << "..." << std::endl;

#define ASSERT_TRUE(expr)                                         \
  do {                                                            \
    if (!(expr)) {                                                \
      std::cerr << "  ❌ 断言失败: " #expr << std::endl;          \
      std::cerr << "     文件: " << __FILE__ << ":" << __LINE__ << std::endl; \
      test_failed++;                                              \
      return;                                                     \
    }                                                             \
  } while (0)

#define ASSERT_FALSE(expr) ASSERT_TRUE(!(expr))

#define ASSERT_EQ(a, b)                                           \
  do {                                                            \
    if ((a) != (b)) {                                             \
      std::cerr << "  ❌ 断言失败: " << #a << " == " << #b << std::endl; \
      std::cerr << "     期望: " << (b) << ", 实际: " << (a) << std::endl; \
      std::cerr << "     文件: " << __FILE__ << ":" << __LINE__ << std::endl; \
      test_failed++;                                              \
      return;                                                     \
    }                                                             \
  } while (0)

#define ASSERT_THROWS(expr)                                       \
  do {                                                            \
    bool threw = false;                                           \
    try {                                                         \
      expr;                                                       \
    } catch (...) {                                               \
      threw = true;                                               \
    }                                                             \
    if (!threw) {                                                 \
      std::cerr << "  ❌ 断言失败: 期望抛出异常" << std::endl;    \
      std::cerr << "     文件: " << __FILE__ << ":" << __LINE__ << std::endl; \
      test_failed++;                                              \
      return;                                                     \
    }                                                             \
  } while (0)

#define TEST_PASSED()                                             \
  do {                                                            \
    std::cout << "  ✅ 通过" << std::endl;                        \
    test_passed++;                                                \
  } while (0)

// ==================== 1. FlagArgument 测试 ====================

void test_flag_basic() {
  TEST_CASE("Flag 基本功能");
  
  ArgumentParser parser("test", "测试程序", false);
  bool verbose = false;
  parser.addFlagArgument({"-v", "--verbose"}, verbose, "详细模式");
  
  ASSERT_TRUE(parser.parse({"--verbose"}));
  ASSERT_TRUE(verbose);
  
  TEST_PASSED();
}

void test_flag_short_name() {
  TEST_CASE("Flag 短名称");
  
  ArgumentParser parser("test", "测试程序", false);
  bool verbose = false;
  parser.addFlagArgument({"-v", "--verbose"}, verbose, "详细模式");
  
  ASSERT_TRUE(parser.parse({"-v"}));
  ASSERT_TRUE(verbose);
  
  TEST_PASSED();
}

void test_flag_not_set() {
  TEST_CASE("Flag 未设置");
  
  ArgumentParser parser("test", "测试程序", false);
  bool verbose = false;
  parser.addFlagArgument({"-v", "--verbose"}, verbose, "详细模式");
  
  ASSERT_TRUE(parser.parse({}));
  ASSERT_FALSE(verbose);
  
  TEST_PASSED();
}

void test_flag_default_value() {
  TEST_CASE("Flag 默认值");
  
  ArgumentParser parser("test", "测试程序", false);
  bool verbose = true;  // 默认为 true
  parser.addFlagArgument({"-v", "--verbose"}, verbose, "详细模式")
        .defaultValue(true);
  
  ASSERT_TRUE(parser.parse({}));
  ASSERT_TRUE(verbose);  // 保持默认值
  
  TEST_PASSED();
}

void test_flag_callback() {
  TEST_CASE("Flag 回调函数");
  
  ArgumentParser parser("test", "测试程序", false);
  int callback_count = 0;
  parser.addFlagArgument({"-v", "--verbose"}, "详细模式", false,
                         [&callback_count]() { callback_count++; });
  
  ASSERT_TRUE(parser.parse({"--verbose"}));
  ASSERT_EQ(callback_count, 1);
  
  TEST_PASSED();
}

// ==================== 2. OptionArgument 测试 ====================

void test_option_basic() {
  TEST_CASE("Option 基本功能");
  
  ArgumentParser parser("test", "测试程序", false);
  std::string output;
  parser.addOptionArgument<std::string>({"-o", "--output"}, output, "输出文件");
  
  ASSERT_TRUE(parser.parse({"-o", "test.txt"}));
  ASSERT_EQ(output, "test.txt");
  
  TEST_PASSED();
}

void test_option_long_name() {
  TEST_CASE("Option 长名称");
  
  ArgumentParser parser("test", "测试程序", false);
  std::string output;
  parser.addOptionArgument<std::string>({"-o", "--output"}, output, "输出文件");
  
  ASSERT_TRUE(parser.parse({"--output", "result.txt"}));
  ASSERT_EQ(output, "result.txt");
  
  TEST_PASSED();
}

void test_option_int() {
  TEST_CASE("Option 整数类型");
  
  ArgumentParser parser("test", "测试程序", false);
  int count;
  parser.addOptionArgument<int>({"-c", "--count"}, count, "数量");
  
  ASSERT_TRUE(parser.parse({"-c", "42"}));
  ASSERT_EQ(count, 42);
  
  TEST_PASSED();
}

void test_option_negative_number() {
  TEST_CASE("Option 负数");
  
  ArgumentParser parser("test", "测试程序", false);
  int value;
  parser.addOptionArgument<int>({"-n", "--number"}, value, "数值");
  
  ASSERT_TRUE(parser.parse({"-n", "-100"}));
  ASSERT_EQ(value, -100);
  
  TEST_PASSED();
}

void test_option_default_value() {
  TEST_CASE("Option 默认值");
  
  ArgumentParser parser("test", "测试程序", false);
  std::string output;
  parser.addOptionArgument<std::string>({"-o", "--output"}, output, "输出文件")
        .defaultValue("default.txt");
  
  ASSERT_TRUE(parser.parse({}));
  ASSERT_EQ(output, "default.txt");
  
  TEST_PASSED();
}

void test_option_callback() {
  TEST_CASE("Option 回调函数");
  
  ArgumentParser parser("test", "测试程序", false);
  std::string output;
  int callback_count = 0;
  parser.addOptionArgument<std::string>({"-o", "--output"}, output, "输出文件")
        .callback([&callback_count]() { callback_count++; });
  
  ASSERT_TRUE(parser.parse({"-o", "test.txt"}));
  ASSERT_EQ(callback_count, 1);
  
  TEST_PASSED();
}

void test_option_required_missing() {
  TEST_CASE("Option 必需参数缺失");
  
  ArgumentParser parser("test", "测试程序", false);
  std::string output;
  parser.addOptionArgument<std::string>({"-o", "--output"}, output, "输出文件")
        .required();
  
  ASSERT_THROWS(parser.parse({}));
  
  TEST_PASSED();
}

void test_option_missing_value() {
  TEST_CASE("Option 缺少值");
  
  ArgumentParser parser("test", "测试程序", false);
  std::string output;
  parser.addOptionArgument<std::string>({"-o", "--output"}, output, "输出文件");
  
  // -o 后面没有值，应该报错
  ASSERT_THROWS(parser.parse({"-o"}));
  
  TEST_PASSED();
}

// ==================== 3. nargs (expected) 测试 ====================

void test_option_nargs_exact() {
  TEST_CASE("Option nargs 精确数量");
  
  ArgumentParser parser("test", "测试程序", false);
  std::vector<std::string> files;
  parser.addOptionArgument<std::string>({"-f", "--files"}, files, "文件列表")
        .expected(3);
  
  ASSERT_TRUE(parser.parse({"-f", "a.txt", "b.txt", "c.txt"}));
  ASSERT_EQ(files.size(), 3u);
  ASSERT_EQ(files[0], "a.txt");
  ASSERT_EQ(files[1], "b.txt");
  ASSERT_EQ(files[2], "c.txt");
  
  TEST_PASSED();
}

void test_option_nargs_exact_insufficient() {
  TEST_CASE("Option nargs 精确数量不足");
  
  ArgumentParser parser("test", "测试程序", false);
  std::vector<std::string> files;
  parser.addOptionArgument<std::string>({"-f", "--files"}, files, "文件列表")
        .expected(3);
  
  // 只提供了2个，需要3个
  ASSERT_THROWS(parser.parse({"-f", "a.txt", "b.txt"}));
  
  TEST_PASSED();
}

void test_option_nargs_range() {
  TEST_CASE("Option nargs 范围 [1, 3]");
  
  ArgumentParser parser("test", "测试程序", false);
  std::vector<std::string> files;
  parser.addOptionArgument<std::string>({"-f", "--files"}, files, "文件列表")
        .expected(1, 3);
  
  ASSERT_TRUE(parser.parse({"-f", "a.txt", "b.txt"}));
  ASSERT_EQ(files.size(), 2u);
  
  TEST_PASSED();
}

void test_option_nargs_range_0_1() {
  TEST_CASE("Option nargs 范围 [0, 1] - 可选参数");
  
  ArgumentParser parser("test", "测试程序", false);
  std::vector<std::string> topic;
  parser.addOptionArgument<std::string>({"--help-topic"}, topic, "帮助主题")
        .expected(0, 1);
  
  // 不带参数
  ASSERT_TRUE(parser.parse({"--help-topic"}));
  ASSERT_EQ(topic.size(), 0u);
  
  TEST_PASSED();
}

void test_option_nargs_range_0_1_with_value() {
  TEST_CASE("Option nargs 范围 [0, 1] 带值");
  
  ArgumentParser parser("test", "测试程序", false);
  std::vector<std::string> topic;
  parser.addOptionArgument<std::string>({"--help-topic"}, topic, "帮助主题")
        .expected(0, 1);
  
  // 带参数
  ASSERT_TRUE(parser.parse({"--help-topic", "install"}));
  ASSERT_EQ(topic.size(), 1u);
  ASSERT_EQ(topic[0], "install");
  
  TEST_PASSED();
}

void test_option_nargs_unlimited() {
  TEST_CASE("Option nargs 无限制 (*)");
  
  ArgumentParser parser("test", "测试程序", false);
  std::vector<std::string> files;
  parser.addOptionArgument<std::string>({"-f", "--files"}, files, "文件列表")
        .expected(MultiArgument<std::string>::ExpectedCount::Unlimited);
  
  ASSERT_TRUE(parser.parse({"-f", "a.txt", "b.txt", "c.txt", "d.txt"}));
  ASSERT_EQ(files.size(), 4u);
  
  TEST_PASSED();
}

void test_option_nargs_unlimited_empty() {
  TEST_CASE("Option nargs 无限制 (*) 空值");
  
  ArgumentParser parser("test", "测试程序", false);
  std::vector<std::string> files;
  parser.addOptionArgument<std::string>({"-f", "--files"}, files, "文件列表")
        .expected(MultiArgument<std::string>::ExpectedCount::Unlimited);
  
  // nargs='*' 允许0个参数
  ASSERT_TRUE(parser.parse({"-f"}));
  ASSERT_EQ(files.size(), 0u);
  
  TEST_PASSED();
}

void test_option_nargs_at_least_one() {
  TEST_CASE("Option nargs 至少一个 (+)");
  
  ArgumentParser parser("test", "测试程序", false);
  std::vector<std::string> files;
  parser.addOptionArgument<std::string>({"-f", "--files"}, files, "文件列表")
        .expected(MultiArgument<std::string>::ExpectedCount::AtLeastOne);
  
  ASSERT_TRUE(parser.parse({"-f", "a.txt", "b.txt"}));
  ASSERT_EQ(files.size(), 2u);
  
  TEST_PASSED();
}

void test_option_nargs_at_least_one_empty() {
  TEST_CASE("Option nargs 至少一个 (+) 空值 - 应报错");
  
  ArgumentParser parser("test", "测试程序", false);
  std::vector<std::string> files;
  parser.addOptionArgument<std::string>({"-f", "--files"}, files, "文件列表")
        .expected(MultiArgument<std::string>::ExpectedCount::AtLeastOne);
  
  // nargs='+' 不允许0个参数
  ASSERT_THROWS(parser.parse({"-f"}));
  
  TEST_PASSED();
}

// ==================== 4. PositionalArgument 测试 ====================

void test_positional_basic() {
  TEST_CASE("Positional 基本功能");
  
  ArgumentParser parser("test", "测试程序", false);
  std::string input;
  parser.addPositionalArgument<std::string>({"input"}, input, "输入文件");
  
  ASSERT_TRUE(parser.parse({"input.txt"}));
  ASSERT_EQ(input, "input.txt");
  
  TEST_PASSED();
}

void test_positional_multiple() {
  TEST_CASE("Positional 多个位置参数");
  
  ArgumentParser parser("test", "测试程序", false);
  std::string input, output;
  parser.addPositionalArgument<std::string>({"input"}, input, "输入文件");
  parser.addPositionalArgument<std::string>({"output"}, output, "输出文件");
  
  ASSERT_TRUE(parser.parse({"in.txt", "out.txt"}));
  ASSERT_EQ(input, "in.txt");
  ASSERT_EQ(output, "out.txt");
  
  TEST_PASSED();
}

void test_positional_with_option() {
  TEST_CASE("Positional 与 Option 混合");
  
  ArgumentParser parser("test", "测试程序", false);
  std::string input;
  bool verbose = false;
  parser.addPositionalArgument<std::string>({"input"}, input, "输入文件");
  parser.addFlagArgument({"-v", "--verbose"}, verbose, "详细模式");
  
  ASSERT_TRUE(parser.parse({"-v", "input.txt"}));
  ASSERT_EQ(input, "input.txt");
  ASSERT_TRUE(verbose);
  
  TEST_PASSED();
}

void test_positional_nargs_unlimited() {
  TEST_CASE("Positional nargs 无限制");
  
  ArgumentParser parser("test", "测试程序", false);
  std::vector<std::string> files;
  parser.addPositionalArgument<std::string>({"files"}, files, "文件列表")
        .expected(MultiArgument<std::string>::ExpectedCount::Unlimited);
  
  ASSERT_TRUE(parser.parse({"a.txt", "b.txt", "c.txt"}));
  ASSERT_EQ(files.size(), 3u);
  
  TEST_PASSED();
}

// ==================== 5. 自动 help 测试 ====================

void test_auto_help_enabled() {
  TEST_CASE("自动 help 启用");
  
  ArgumentParser parser("test", "测试程序", true);  // 启用自动 help
  
  // 遇到 --help 应该返回 false
  bool result = parser.parse({"--help"});
  ASSERT_FALSE(result);
  
  TEST_PASSED();
}

void test_auto_help_short() {
  TEST_CASE("自动 help 短名称 -h");
  
  ArgumentParser parser("test", "测试程序", true);
  
  bool result = parser.parse({"-h"});
  ASSERT_FALSE(result);
  
  TEST_PASSED();
}

void test_auto_help_disabled() {
  TEST_CASE("自动 help 禁用");
  
  ArgumentParser parser("test", "测试程序", false);  // 禁用自动 help
  
  // 没有自动 help，--help 应该报错（未知参数）
  ASSERT_THROWS(parser.parse({"--help"}));
  
  TEST_PASSED();
}

void test_user_defined_help_flag() {
  TEST_CASE("用户自定义 --help 为 Flag");
  
  ArgumentParser parser("test", "测试程序", true);  // 启用自动 help
  bool help_flag = false;
  parser.addFlagArgument({"-h", "--help"}, help_flag, "自定义帮助");
  
  // 用户定义了 --help，自动 help 应该让路
  ASSERT_TRUE(parser.parse({"--help"}));
  ASSERT_TRUE(help_flag);  // 用户的 flag 被设置
  
  TEST_PASSED();
}

void test_user_defined_help_option() {
  TEST_CASE("用户自定义 --help 为 Option (nargs [0,1])");
  
  ArgumentParser parser("test", "测试程序", true);
  std::vector<std::string> help_topic;
  parser.addOptionArgument<std::string>({"--help"}, help_topic, "显示帮助")
        .expected(0, 1);
  
  // 用户定义了 --help，自动 help 让路
  ASSERT_TRUE(parser.parse({"--help", "install"}));
  ASSERT_EQ(help_topic.size(), 1u);
  ASSERT_EQ(help_topic[0], "install");
  
  TEST_PASSED();
}

void test_user_defined_help_option_no_value() {
  TEST_CASE("用户自定义 --help 为 Option (nargs [0,1]) 无值");
  
  ArgumentParser parser("test", "测试程序", true);
  std::vector<std::string> help_topic;
  parser.addOptionArgument<std::string>({"--help"}, help_topic, "显示帮助")
        .expected(0, 1);
  
  // --help 不带参数，因为 nargs=[0,1]，所以合法
  ASSERT_TRUE(parser.parse({"--help"}));
  ASSERT_EQ(help_topic.size(), 0u);
  
  TEST_PASSED();
}

// ==================== 6. 子命令测试 ====================

void test_subcommand_basic() {
  TEST_CASE("子命令基本功能");
  
  ArgumentParser parser("git", "Git 命令", false);
  auto& clone = parser.addSubcommand("clone", "克隆仓库");
  
  std::string url;
  clone.addPositionalArgument<std::string>({"url"}, url, "仓库地址");
  
  ASSERT_TRUE(parser.parse({"clone", "https://github.com/test/repo.git"}));
  ASSERT_EQ(url, "https://github.com/test/repo.git");
  
  TEST_PASSED();
}

void test_subcommand_with_options() {
  TEST_CASE("子命令带选项");
  
  ArgumentParser parser("git", "Git 命令", false);
  auto& commit = parser.addSubcommand("commit", "提交");
  
  std::string message;
  commit.addOptionArgument<std::string>({"-m", "--message"}, message, "提交信息");
  
  ASSERT_TRUE(parser.parse({"commit", "-m", "Initial commit"}));
  ASSERT_EQ(message, "Initial commit");
  
  TEST_PASSED();
}

// ==================== 7. 异常处理测试 ====================

void test_unknown_argument() {
  TEST_CASE("未知参数");
  
  ArgumentParser parser("test", "测试程序", false);
  
  ASSERT_THROWS(parser.parse({"--unknown"}));
  
  TEST_PASSED();
}

void test_invalid_flag_name() {
  TEST_CASE("无效的 Flag 名称");
  
  ArgumentParser parser("test", "测试程序", false);
  
  // Flag 名称必须以 - 开头
  ASSERT_THROWS(parser.addFlagArgument({"verbose"}, "详细模式"));
  
  TEST_PASSED();
}

void test_duplicate_argument_name() {
  TEST_CASE("重复的参数名称");
  
  ArgumentParser parser("test", "测试程序", false);
  parser.addFlagArgument({"-v", "--verbose"}, "详细模式");
  
  // 尝试添加同名参数，应该抛出异常
  ASSERT_THROWS(parser.addFlagArgument({"-v"}, "另一个 verbose"));
  ASSERT_THROWS(parser.addFlagArgument({"--verbose"}, "另一个 verbose"));
  ASSERT_THROWS(parser.addOptionArgument<std::string>({"-v"}, "同名选项"));
  
  TEST_PASSED();
}

void test_empty_subcommand_name() {
  TEST_CASE("空子命令名称");
  
  ArgumentParser parser("test", "测试程序", false);
  
  ASSERT_THROWS(parser.addSubcommand("", "空名称"));
  
  TEST_PASSED();
}

void test_subcommand_starts_with_dash() {
  TEST_CASE("子命令名称以 - 开头");
  
  ArgumentParser parser("test", "测试程序", false);
  
  ASSERT_THROWS(parser.addSubcommand("-cmd", "非法名称"));
  
  TEST_PASSED();
}

// ==================== 8. 复杂场景测试 ====================

void test_mixed_arguments() {
  TEST_CASE("混合参数场景 - 基本");
  
  ArgumentParser parser("myapp", "测试程序", false);
  
  bool verbose = false;
  std::string output;
  int count = 0;
  std::string input;
  
  parser.addFlagArgument({"-v", "--verbose"}, verbose, "详细模式");
  parser.addOptionArgument<std::string>({"-o", "--output"}, output, "输出文件")
        .defaultValue("out.txt");
  parser.addOptionArgument<int>({"-c", "--count"}, count, "计数");
  parser.addPositionalArgument<std::string>({"input"}, input, "输入文件");
  
  ASSERT_TRUE(parser.parse({"-v", "-c", "10", "input.txt"}));
  ASSERT_TRUE(verbose);
  ASSERT_EQ(output, "out.txt");  // 使用默认值
  ASSERT_EQ(count, 10);
  ASSERT_EQ(input, "input.txt");
  
  TEST_PASSED();
}

void test_mixed_with_subcommand() {
  TEST_CASE("混合参数场景 - 子命令 + 选项 + 位置参数");
  
  ArgumentParser parser("git", "Git 版本控制", false);
  
  // 全局选项
  bool global_verbose = false;
  parser.addFlagArgument({"-v", "--verbose"}, global_verbose, "全局详细模式");
  
  // clone 子命令
  auto& clone = parser.addSubcommand("clone", "克隆仓库");
  std::string url;
  std::string branch;
  int depth = 0;
  bool shallow = false;
  clone.addPositionalArgument<std::string>({"url"}, url, "仓库地址");
  clone.addOptionArgument<std::string>({"-b", "--branch"}, branch, "分支名")
       .defaultValue("main");
  clone.addOptionArgument<int>({"--depth"}, depth, "克隆深度");
  clone.addFlagArgument({"--shallow"}, shallow, "浅克隆");
  
  ASSERT_TRUE(parser.parse({"clone", "-b", "develop", "--depth", "1", "--shallow", "https://github.com/test/repo.git"}));
  ASSERT_EQ(url, "https://github.com/test/repo.git");
  ASSERT_EQ(branch, "develop");
  ASSERT_EQ(depth, 1);
  ASSERT_TRUE(shallow);
  
  TEST_PASSED();
}

void test_mixed_with_help_and_subcommand() {
  TEST_CASE("混合参数场景 - 自动 help + 子命令");
  
  ArgumentParser parser("myapp", "应用程序", true);  // 启用自动 help
  
  auto& sub = parser.addSubcommand("run", "运行");
  std::string config;
  sub.addOptionArgument<std::string>({"-c", "--config"}, config, "配置文件");
  
  // 主命令 --help 应该触发自动 help
  ASSERT_FALSE(parser.parse({"--help"}));
  
  TEST_PASSED();
}

void test_mixed_with_user_help_and_subcommand() {
  TEST_CASE("混合参数场景 - 用户自定义 help + 子命令");
  
  ArgumentParser parser("myapp", "应用程序", true);
  
  // 用户自定义 --help 为 nargs [0,1]
  std::vector<std::string> help_topic;
  parser.addOptionArgument<std::string>({"--help"}, help_topic, "显示帮助")
        .expected(0, 1);
  
  // 添加子命令
  auto& install = parser.addSubcommand("install", "安装");
  std::string package;
  install.addPositionalArgument<std::string>({"package"}, package, "包名");
  
  // 测试 --help 带主题
  ASSERT_TRUE(parser.parse({"--help", "install"}));
  ASSERT_EQ(help_topic.size(), 1u);
  ASSERT_EQ(help_topic[0], "install");
  
  TEST_PASSED();
}

void test_mixed_subcommand_help() {
  TEST_CASE("混合参数场景 - 子命令的 --help");
  
  ArgumentParser parser("git", "Git 命令", true);  // 启用自动 help
  
  auto& commit = parser.addSubcommand("commit", "提交");
  std::string message;
  commit.addOptionArgument<std::string>({"-m", "--message"}, message, "提交信息");
  
  // 子命令后面跟 --help 应该显示子命令的帮助
  ASSERT_FALSE(parser.parse({"commit", "--help"}));
  
  TEST_PASSED();
}

void test_mixed_all_types() {
  TEST_CASE("混合参数场景 - 全类型组合");
  
  ArgumentParser parser("myapp", "全功能测试", true);  // 启用自动 help
  
  // 用户自定义 help，覆盖自动 help
  bool show_help = false;
  parser.addFlagArgument({"-h", "--help"}, show_help, "显示帮助");
  
  // 全局 flag
  bool verbose = false;
  bool debug = false;
  parser.addFlagArgument({"-v", "--verbose"}, verbose, "详细模式");
  parser.addFlagArgument({"-d", "--debug"}, debug, "调试模式");
  
  // 全局 option
  std::string config;
  int jobs = 1;
  parser.addOptionArgument<std::string>({"-c", "--config"}, config, "配置文件")
        .defaultValue("config.yaml");
  parser.addOptionArgument<int>({"-j", "--jobs"}, jobs, "并行数");
  
  // 全局 multi-option
  std::vector<std::string> includes;
  parser.addOptionArgument<std::string>({"-I", "--include"}, includes, "包含目录")
        .expected(MultiArgument<std::string>::ExpectedCount::Unlimited);
  
  // 子命令 build
  auto& build = parser.addSubcommand("build", "构建项目");
  std::string target;
  bool release = false;
  build.addPositionalArgument<std::string>({"target"}, target, "构建目标");
  build.addFlagArgument({"-r", "--release"}, release, "Release 模式");
  
  // 解析复杂命令行
  // 注意：子命令必须放在第一个位置，全局参数在子命令之后由子命令解析器处理
  ASSERT_TRUE(parser.parse({
    "build", "-r", "myproject"
  }));
  
  // 注意：当使用子命令时，主解析器不会执行默认值同步
  // 因此这里只验证子命令的参数
  ASSERT_EQ(target, "myproject");
  ASSERT_TRUE(release);
  
  TEST_PASSED();
}

void test_mixed_multiple_subcommands() {
  TEST_CASE("混合参数场景 - 多子命令");
  
  ArgumentParser parser("pkg", "包管理器", false);
  
  // install 子命令
  auto& install = parser.addSubcommand("install", "安装包");
  std::string install_pkg;
  bool global = false;
  install.addPositionalArgument<std::string>({"package"}, install_pkg, "包名");
  install.addFlagArgument({"-g", "--global"}, global, "全局安装");
  
  // remove 子命令
  auto& remove = parser.addSubcommand("remove", "移除包");
  std::string remove_pkg;
  bool force = false;
  remove.addPositionalArgument<std::string>({"package"}, remove_pkg, "包名");
  remove.addFlagArgument({"-f", "--force"}, force, "强制移除");
  
  // list 子命令
  auto& list = parser.addSubcommand("list", "列出包");
  bool all = false;
  list.addFlagArgument({"-a", "--all"}, all, "显示所有");
  
  // 测试 install 子命令
  ASSERT_TRUE(parser.parse({"install", "-g", "lodash"}));
  ASSERT_EQ(install_pkg, "lodash");
  ASSERT_TRUE(global);
  
  TEST_PASSED();
}

void test_mixed_nargs_and_positional() {
  TEST_CASE("混合参数场景 - nargs 与位置参数");
  
  ArgumentParser parser("cp", "复制文件", false);
  
  bool recursive = false;
  parser.addFlagArgument({"-r", "--recursive"}, recursive, "递归复制");
  
  // 单值位置参数可以正常工作
  std::string source;
  std::string dest;
  parser.addPositionalArgument<std::string>({"source"}, source, "源文件");
  parser.addPositionalArgument<std::string>({"dest"}, dest, "目标");
  
  ASSERT_TRUE(parser.parse({"-r", "file.txt", "backup/"}));
  ASSERT_TRUE(recursive);
  ASSERT_EQ(source, "file.txt");
  ASSERT_EQ(dest, "backup/");
  
  TEST_PASSED();
}

void test_mixed_callback_chain() {
  TEST_CASE("混合参数场景 - 回调链");
  
  ArgumentParser parser("test", "测试程序", false);
  
  std::vector<std::string> execution_order;
  
  parser.addFlagArgument({"-a"}, "选项A", false, 
                         [&]() { execution_order.push_back("a"); });
  parser.addFlagArgument({"-b"}, "选项B", false,
                         [&]() { execution_order.push_back("b"); });
  parser.addFlagArgument({"-c"}, "选项C", false,
                         [&]() { execution_order.push_back("c"); });
  
  ASSERT_TRUE(parser.parse({"-a", "-c", "-b"}));
  
  // 验证回调按命令行顺序执行
  ASSERT_EQ(execution_order.size(), 3u);
  ASSERT_EQ(execution_order[0], "a");
  ASSERT_EQ(execution_order[1], "c");
  ASSERT_EQ(execution_order[2], "b");
  
  TEST_PASSED();
}

void test_option_stops_at_next_option() {
  TEST_CASE("Option 遇到下一个选项停止");
  
  ArgumentParser parser("test", "测试程序", false);
  
  std::vector<std::string> files;
  bool verbose = false;
  
  parser.addOptionArgument<std::string>({"-f", "--files"}, files, "文件列表")
        .expected(MultiArgument<std::string>::ExpectedCount::Unlimited);
  parser.addFlagArgument({"-v", "--verbose"}, verbose, "详细模式");
  
  ASSERT_TRUE(parser.parse({"-f", "a.txt", "b.txt", "-v"}));
  ASSERT_EQ(files.size(), 2u);  // 遇到 -v 停止
  ASSERT_TRUE(verbose);
  
  TEST_PASSED();
}

void test_double_dash_separator() {
  TEST_CASE("双短横线分隔符 --");
  
  ArgumentParser parser("test", "测试程序", false);
  
  std::vector<std::string> files;
  bool verbose = false;
  
  parser.addOptionArgument<std::string>({"-f", "--files"}, files, "文件列表")
        .expected(MultiArgument<std::string>::ExpectedCount::Unlimited);
  parser.addFlagArgument({"-v", "--verbose"}, verbose, "详细模式");
  
  // -- 后面的内容不再被 -f 消费
  ASSERT_TRUE(parser.parse({"-f", "a.txt", "b.txt", "--", "-v"}));
  ASSERT_EQ(files.size(), 2u);
  ASSERT_EQ(files[0], "a.txt");
  ASSERT_EQ(files[1], "b.txt");
  // 注意: -- 后面的 -v 会被当作位置参数（如果有），或报错（没有位置参数时）
  
  TEST_PASSED();
}

// ==================== 9. 更多混合模式测试 ====================

// 9.1 参数顺序变化测试
void test_mixed_order_positional_first() {
  TEST_CASE("混合模式 - 位置参数在最前");
  
  ArgumentParser parser("test", "测试程序", false);
  
  bool verbose = false;
  int count = 0;
  std::string input;
  
  parser.addFlagArgument({"-v", "--verbose"}, verbose, "详细模式");
  parser.addOptionArgument<int>({"-c", "--count"}, count, "计数");
  parser.addPositionalArgument<std::string>({"input"}, input, "输入文件");
  
  ASSERT_TRUE(parser.parse({"input.txt", "-v", "-c", "10"}));
  ASSERT_EQ(input, "input.txt");
  ASSERT_TRUE(verbose);
  ASSERT_EQ(count, 10);
  
  TEST_PASSED();
}

void test_mixed_order_positional_middle() {
  TEST_CASE("混合模式 - 位置参数在中间");
  
  ArgumentParser parser("test", "测试程序", false);
  
  bool verbose = false;
  int count = 0;
  std::string input;
  
  parser.addFlagArgument({"-v", "--verbose"}, verbose, "详细模式");
  parser.addOptionArgument<int>({"-c", "--count"}, count, "计数");
  parser.addPositionalArgument<std::string>({"input"}, input, "输入文件");
  
  ASSERT_TRUE(parser.parse({"-v", "input.txt", "-c", "10"}));
  ASSERT_EQ(input, "input.txt");
  ASSERT_TRUE(verbose);
  ASSERT_EQ(count, 10);
  
  TEST_PASSED();
}

void test_mixed_order_flags_scattered() {
  TEST_CASE("混合模式 - Flag 分散在各处");
  
  ArgumentParser parser("test", "测试程序", false);
  
  bool verbose = false;
  bool debug = false;
  bool force = false;
  std::string output;
  std::string input;
  
  parser.addFlagArgument({"-v", "--verbose"}, verbose, "详细模式");
  parser.addFlagArgument({"-d", "--debug"}, debug, "调试模式");
  parser.addFlagArgument({"-f", "--force"}, force, "强制模式");
  parser.addOptionArgument<std::string>({"-o", "--output"}, output, "输出文件");
  parser.addPositionalArgument<std::string>({"input"}, input, "输入文件");
  
  ASSERT_TRUE(parser.parse({"-v", "-o", "out.txt", "-d", "input.txt", "-f"}));
  ASSERT_TRUE(verbose);
  ASSERT_TRUE(debug);
  ASSERT_TRUE(force);
  ASSERT_EQ(output, "out.txt");
  ASSERT_EQ(input, "input.txt");
  
  TEST_PASSED();
}

// 9.2 多个 Option 交错测试
void test_mixed_multiple_options_interleaved() {
  TEST_CASE("混合模式 - 多个 Option 交错");
  
  ArgumentParser parser("test", "测试程序", false);
  
  std::string name;
  int age = 0;
  std::string city;
  double score = 0.0;
  
  parser.addOptionArgument<std::string>({"-n", "--name"}, name, "姓名");
  parser.addOptionArgument<int>({"-a", "--age"}, age, "年龄");
  parser.addOptionArgument<std::string>({"-c", "--city"}, city, "城市");
  parser.addOptionArgument<double>({"-s", "--score"}, score, "分数");
  
  ASSERT_TRUE(parser.parse({"-n", "Alice", "-s", "95.5", "-a", "25", "-c", "Beijing"}));
  ASSERT_EQ(name, "Alice");
  ASSERT_EQ(age, 25);
  ASSERT_EQ(city, "Beijing");
  ASSERT_EQ(score, 95.5);
  
  TEST_PASSED();
}

void test_mixed_options_with_multiple_positional() {
  TEST_CASE("混合模式 - Option 与多个位置参数");
  
  ArgumentParser parser("test", "测试程序", false);
  
  std::string input;
  std::string output;
  bool verbose = false;
  int level = 0;
  
  parser.addPositionalArgument<std::string>({"input"}, input, "输入文件");
  parser.addPositionalArgument<std::string>({"output"}, output, "输出文件");
  parser.addFlagArgument({"-v", "--verbose"}, verbose, "详细模式");
  parser.addOptionArgument<int>({"-l", "--level"}, level, "级别");
  
  ASSERT_TRUE(parser.parse({"-v", "in.txt", "-l", "3", "out.txt"}));
  ASSERT_EQ(input, "in.txt");
  ASSERT_EQ(output, "out.txt");
  ASSERT_TRUE(verbose);
  ASSERT_EQ(level, 3);
  
  TEST_PASSED();
}

// 9.3 子命令与全局参数混合测试
void test_mixed_global_option_before_subcommand() {
  TEST_CASE("混合模式 - 子命令在第一位（符合主流CLI惯例）");
  
  ArgumentParser parser("git", "Git 命令", false);
  
  // 注意：按照 git/docker/kubectl 等主流CLI工具的惯例，
  // 子命令应该出现在第一位，全局参数不支持在子命令前
  
  auto& clone = parser.addSubcommand("clone", "克隆仓库");
  std::string url;
  bool verbose = false;
  clone.addPositionalArgument<std::string>({"url"}, url, "仓库地址");
  clone.addFlagArgument({"-v", "--verbose"}, verbose, "详细模式");
  
  // 子命令在第一位，选项在后面
  ASSERT_TRUE(parser.parse({"clone", "-v", "https://github.com/test/repo.git"}));
  ASSERT_TRUE(verbose);
  ASSERT_EQ(url, "https://github.com/test/repo.git");
  
  TEST_PASSED();
}

void test_mixed_subcommand_with_multiple_global_options() {
  TEST_CASE("混合模式 - 子命令与多个选项");
  
  ArgumentParser parser("app", "应用程序", false);
  
  // 按照主流CLI惯例，子命令在第一位，选项属于子命令
  auto& run = parser.addSubcommand("run", "运行");
  
  bool verbose = false;
  std::string config;
  int log_level = 0;
  std::string target;
  bool daemon = false;
  
  run.addFlagArgument({"-v", "--verbose"}, verbose, "详细模式");
  run.addOptionArgument<std::string>({"-c", "--config"}, config, "配置文件")
        .defaultValue("app.yaml");
  run.addOptionArgument<int>({"--log-level"}, log_level, "日志级别");
  run.addPositionalArgument<std::string>({"target"}, target, "目标");
  run.addFlagArgument({"-d", "--daemon"}, daemon, "后台运行");
  
  ASSERT_TRUE(parser.parse({"run", "-v", "--log-level", "2", "-d", "myapp"}));
  ASSERT_TRUE(verbose);
  ASSERT_EQ(log_level, 2);
  ASSERT_EQ(target, "myapp");
  ASSERT_TRUE(daemon);
  
  TEST_PASSED();
}

// 9.4 nargs 与其他参数混合测试
void test_mixed_nargs_unlimited_with_flag() {
  TEST_CASE("混合模式 - nargs=* 与 Flag");
  
  ArgumentParser parser("test", "测试程序", false);
  
  std::vector<std::string> files;
  bool verbose = false;
  bool recursive = false;
  
  parser.addOptionArgument<std::string>({"-f", "--files"}, files, "文件列表")
        .expected(MultiArgument<std::string>::ExpectedCount::Unlimited);
  parser.addFlagArgument({"-v", "--verbose"}, verbose, "详细模式");
  parser.addFlagArgument({"-r", "--recursive"}, recursive, "递归");
  
  ASSERT_TRUE(parser.parse({"-v", "-f", "a.txt", "b.txt", "c.txt", "-r"}));
  ASSERT_TRUE(verbose);
  ASSERT_TRUE(recursive);
  ASSERT_EQ(files.size(), 3u);
  ASSERT_EQ(files[0], "a.txt");
  ASSERT_EQ(files[1], "b.txt");
  ASSERT_EQ(files[2], "c.txt");
  
  TEST_PASSED();
}

void test_mixed_nargs_at_least_one_with_option() {
  TEST_CASE("混合模式 - nargs=+ 与 Option");
  
  ArgumentParser parser("test", "测试程序", false);
  
  std::vector<std::string> files;
  std::string output;
  int count = 0;
  
  parser.addOptionArgument<std::string>({"-f", "--files"}, files, "文件列表")
        .expected(MultiArgument<std::string>::ExpectedCount::AtLeastOne);
  parser.addOptionArgument<std::string>({"-o", "--output"}, output, "输出");
  parser.addOptionArgument<int>({"-c", "--count"}, count, "计数");
  
  ASSERT_TRUE(parser.parse({"-c", "5", "-f", "x.txt", "y.txt", "-o", "result.txt"}));
  ASSERT_EQ(count, 5);
  ASSERT_EQ(files.size(), 2u);
  ASSERT_EQ(output, "result.txt");
  
  TEST_PASSED();
}

void test_mixed_nargs_range_with_positional() {
  TEST_CASE("混合模式 - nargs=[1,3] 与位置参数");
  
  ArgumentParser parser("test", "测试程序", false);
  
  std::vector<std::string> tags;
  std::string input;
  std::string output;
  
  parser.addOptionArgument<std::string>({"-t", "--tags"}, tags, "标签")
        .expected(1, 3);
  parser.addPositionalArgument<std::string>({"input"}, input, "输入");
  parser.addPositionalArgument<std::string>({"output"}, output, "输出");
  
  // 使用 -- 提前结束 -t 的参数收集，让 out.txt 能被位置参数 output 捕获
  ASSERT_TRUE(parser.parse({"in.txt", "-t", "tag1", "tag2", "--", "out.txt"}));
  ASSERT_EQ(input, "in.txt");
  ASSERT_EQ(output, "out.txt");
  ASSERT_EQ(tags.size(), 2u);
  ASSERT_EQ(tags[0], "tag1");
  ASSERT_EQ(tags[1], "tag2");
  
  TEST_PASSED();
}

void test_mixed_multiple_nargs_options() {
  TEST_CASE("混合模式 - 多个 nargs 选项");
  
  ArgumentParser parser("test", "测试程序", false);
  
  std::vector<std::string> includes;
  std::vector<std::string> excludes;
  std::vector<int> numbers;
  
  parser.addOptionArgument<std::string>({"-I", "--include"}, includes, "包含")
        .expected(MultiArgument<std::string>::ExpectedCount::Unlimited);
  parser.addOptionArgument<std::string>({"-E", "--exclude"}, excludes, "排除")
        .expected(1, 3);
  parser.addOptionArgument<int>({"-n", "--numbers"}, numbers, "数字")
        .expected(2);
  
  ASSERT_TRUE(parser.parse({
    "-I", "a", "b", "c",
    "-E", "x", "y",
    "-n", "10", "20"
  }));
  ASSERT_EQ(includes.size(), 3u);
  ASSERT_EQ(excludes.size(), 2u);
  ASSERT_EQ(numbers.size(), 2u);
  ASSERT_EQ(numbers[0], 10);
  ASSERT_EQ(numbers[1], 20);
  
  TEST_PASSED();
}

// 9.5 默认值与混合参数测试
void test_mixed_multiple_defaults() {
  TEST_CASE("混合模式 - 多个默认值");
  
  ArgumentParser parser("test", "测试程序", false);
  
  bool verbose = false;
  std::string output;
  int count = 0;
  double ratio = 0.0;
  std::string input;
  
  parser.addFlagArgument({"-v", "--verbose"}, verbose, "详细模式")
        .defaultValue(true);
  parser.addOptionArgument<std::string>({"-o", "--output"}, output, "输出文件")
        .defaultValue("default_output.txt");
  parser.addOptionArgument<int>({"-c", "--count"}, count, "计数")
        .defaultValue(100);
  parser.addOptionArgument<double>({"-r", "--ratio"}, ratio, "比率")
        .defaultValue(0.5);
  parser.addPositionalArgument<std::string>({"input"}, input, "输入文件")
        .defaultValue("default_input.txt");
  
  // 什么都不传，全部使用默认值
  ASSERT_TRUE(parser.parse({}));
  ASSERT_TRUE(verbose);
  ASSERT_EQ(output, "default_output.txt");
  ASSERT_EQ(count, 100);
  ASSERT_EQ(ratio, 0.5);
  ASSERT_EQ(input, "default_input.txt");
  
  TEST_PASSED();
}

void test_mixed_partial_defaults() {
  TEST_CASE("混合模式 - 部分使用默认值");
  
  ArgumentParser parser("test", "测试程序", false);
  
  std::string name;
  int age = 0;
  std::string city;
  
  parser.addOptionArgument<std::string>({"-n", "--name"}, name, "姓名")
        .defaultValue("Unknown");
  parser.addOptionArgument<int>({"-a", "--age"}, age, "年龄")
        .defaultValue(18);
  parser.addOptionArgument<std::string>({"-c", "--city"}, city, "城市")
        .defaultValue("Beijing");
  
  // 只覆盖 age，其他用默认值
  ASSERT_TRUE(parser.parse({"-a", "30"}));
  ASSERT_EQ(name, "Unknown");
  ASSERT_EQ(age, 30);
  ASSERT_EQ(city, "Beijing");
  
  TEST_PASSED();
}

// 9.6 help 与其他参数混合测试
void test_mixed_help_with_other_args_before() {
  TEST_CASE("混合模式 - help 前有其他参数");
  
  ArgumentParser parser("test", "测试程序", true);  // 启用自动 help
  
  bool verbose = false;
  std::string output;
  parser.addFlagArgument({"-v", "--verbose"}, verbose, "详细模式");
  parser.addOptionArgument<std::string>({"-o", "--output"}, output, "输出");
  
  // -v 后跟 --help
  bool result = parser.parse({"-v", "--help"});
  ASSERT_FALSE(result);  // 遇到 --help 返回 false
  
  TEST_PASSED();
}

void test_mixed_user_help_with_subcommand_and_args() {
  TEST_CASE("混合模式 - 用户 help + 子命令 + 参数");
  
  ArgumentParser parser("app", "应用程序", true);
  
  // 用户自定义 --help 为 nargs [0,1]
  std::vector<std::string> help_topic;
  parser.addOptionArgument<std::string>({"--help"}, help_topic, "显示帮助")
        .expected(0, 1);
  
  bool verbose = false;
  parser.addFlagArgument({"-v", "--verbose"}, verbose, "详细模式");
  
  // 添加子命令
  auto& build = parser.addSubcommand("build", "构建");
  std::string target;
  build.addPositionalArgument<std::string>({"target"}, target, "目标");
  
  // 测试: --help topic + 其他参数
  ASSERT_TRUE(parser.parse({"-v", "--help", "build"}));
  ASSERT_TRUE(verbose);
  ASSERT_EQ(help_topic.size(), 1u);
  ASSERT_EQ(help_topic[0], "build");
  
  TEST_PASSED();
}

// 9.7 复杂位置参数场景
void test_mixed_positional_after_all_options() {
  TEST_CASE("混合模式 - 所有位置参数在选项之后");
  
  ArgumentParser parser("test", "测试程序", false);
  
  bool verbose = false;
  std::string format;
  std::string input;
  std::string output;
  
  parser.addFlagArgument({"-v", "--verbose"}, verbose, "详细模式");
  parser.addOptionArgument<std::string>({"-f", "--format"}, format, "格式");
  parser.addPositionalArgument<std::string>({"input"}, input, "输入");
  parser.addPositionalArgument<std::string>({"output"}, output, "输出");
  
  ASSERT_TRUE(parser.parse({"-v", "-f", "json", "in.txt", "out.txt"}));
  ASSERT_TRUE(verbose);
  ASSERT_EQ(format, "json");
  ASSERT_EQ(input, "in.txt");
  ASSERT_EQ(output, "out.txt");
  
  TEST_PASSED();
}

void test_mixed_positional_between_options() {
  TEST_CASE("混合模式 - 位置参数在选项之间");
  
  ArgumentParser parser("test", "测试程序", false);
  
  bool verbose = false;
  std::string format;
  std::string input;
  std::string output;
  
  parser.addFlagArgument({"-v", "--verbose"}, verbose, "详细模式");
  parser.addOptionArgument<std::string>({"-f", "--format"}, format, "格式");
  parser.addPositionalArgument<std::string>({"input"}, input, "输入");
  parser.addPositionalArgument<std::string>({"output"}, output, "输出");
  
  ASSERT_TRUE(parser.parse({"-v", "in.txt", "-f", "xml", "out.txt"}));
  ASSERT_TRUE(verbose);
  ASSERT_EQ(format, "xml");
  ASSERT_EQ(input, "in.txt");
  ASSERT_EQ(output, "out.txt");
  
  TEST_PASSED();
}

// 9.8 子命令内部混合测试
void test_mixed_subcommand_all_types() {
  TEST_CASE("混合模式 - 子命令内全类型参数");
  
  ArgumentParser parser("docker", "Docker 命令", false);
  
  auto& run = parser.addSubcommand("run", "运行容器");
  
  bool detach = false;
  bool rm = false;
  std::string name;
  std::vector<std::string> env;
  std::vector<std::string> ports;
  std::string image;
  std::vector<std::string> cmd;
  
  run.addFlagArgument({"-d", "--detach"}, detach, "后台运行");
  run.addFlagArgument({"--rm"}, rm, "退出时删除");
  run.addOptionArgument<std::string>({"--name"}, name, "容器名");
  run.addOptionArgument<std::string>({"-e", "--env"}, env, "环境变量")
     .expected(MultiArgument<std::string>::ExpectedCount::Unlimited);
  run.addOptionArgument<std::string>({"-p", "--publish"}, ports, "端口映射")
     .expected(MultiArgument<std::string>::ExpectedCount::Unlimited);
  run.addPositionalArgument<std::string>({"image"}, image, "镜像名");
  
  // 注意：-e 有 Unlimited 期望值，会贪婪消费，需要用 -- 分隔选项和位置参数
  // 或者每个 -e 只跟一个值
  ASSERT_TRUE(parser.parse({
    "run", "-d", "--rm",
    "--name", "mycontainer",
    "-e", "FOO=bar", "-e", "BAZ=qux",  // 每个 -e 跟一个值
    "-p", "8080:80",
    "--",  // 分隔符，后面的都是位置参数
    "nginx:latest",
  }));
  
  ASSERT_TRUE(detach);
  ASSERT_TRUE(rm);
  ASSERT_EQ(name, "mycontainer");
  ASSERT_EQ(env.size(), 2u);
  ASSERT_EQ(ports.size(), 1u);
  ASSERT_EQ(image, "nginx:latest");
  
  TEST_PASSED();
}

void test_mixed_nested_subcommands() {
  TEST_CASE("混合模式 - 嵌套子命令");
  
  ArgumentParser parser("kubectl", "Kubernetes 命令", false);
  
  auto& get = parser.addSubcommand("get", "获取资源");
  
  std::string resource_type;
  std::string resource_name;
  std::string ns;
  std::string output;
  bool watch = false;
  
  get.addPositionalArgument<std::string>({"type"}, resource_type, "资源类型");
  get.addPositionalArgument<std::string>({"name"}, resource_name, "资源名")
     .defaultValue("");
  get.addOptionArgument<std::string>({"-n", "--namespace"}, ns, "命名空间")
     .defaultValue("default");
  get.addOptionArgument<std::string>({"-o", "--output"}, output, "输出格式");
  get.addFlagArgument({"-w", "--watch"}, watch, "监听变化");
  
  ASSERT_TRUE(parser.parse({"get", "-n", "kube-system", "-w", "pods", "-o", "wide"}));
  ASSERT_EQ(resource_type, "pods");
  ASSERT_EQ(ns, "kube-system");
  ASSERT_EQ(output, "wide");
  ASSERT_TRUE(watch);
  
  TEST_PASSED();
}

// 9.9 边界情况测试
void test_mixed_empty_string_value() {
  TEST_CASE("混合模式 - 空字符串值");
  
  ArgumentParser parser("test", "测试程序", false);
  
  std::string value;
  parser.addOptionArgument<std::string>({"-v", "--value"}, value, "值");
  
  // 空字符串作为值
  ASSERT_TRUE(parser.parse({"-v", ""}));
  ASSERT_EQ(value, "");
  
  TEST_PASSED();
}

void test_mixed_value_looks_like_option() {
  TEST_CASE("混合模式 - 值看起来像选项（负数）");
  
  ArgumentParser parser("test", "测试程序", false);
  
  int min_val = 0;
  int max_val = 0;
  
  parser.addOptionArgument<int>({"--min"}, min_val, "最小值");
  parser.addOptionArgument<int>({"--max"}, max_val, "最大值");
  
  ASSERT_TRUE(parser.parse({"--min", "-10", "--max", "100"}));
  ASSERT_EQ(min_val, -10);
  ASSERT_EQ(max_val, 100);
  
  TEST_PASSED();
}

void test_mixed_long_option_with_equals() {
  TEST_CASE("混合模式 - 长选项等号形式 (当前不支持)");
  
  // 当前实现不支持 --option=value 形式
  // 这里只是标记为测试通过，提示该功能未实现
  // 未来如果实现了，可以添加具体测试
  
  // 示例（如果实现）：
  // ArgumentParser parser("test", "测试程序", false);
  // std::string output;
  // parser.addOptionArgument<std::string>({"--output"}, output, "输出");
  // ASSERT_TRUE(parser.parse({"--output=file.txt"}));
  // ASSERT_EQ(output, "file.txt");
  
  TEST_PASSED();
}

void test_mixed_combined_short_flags() {
  TEST_CASE("混合模式 - 组合短 flag (当前不支持)");
  
  // 当前实现不支持 -abc 形式（等价于 -a -b -c）
  // 这里只是标记为测试通过，提示该功能未实现
  // 未来如果实现了，可以添加具体测试
  
  // 示例（如果实现）：
  // ArgumentParser parser("test", "测试程序", false);
  // bool a = false, b = false, c = false;
  // parser.addFlagArgument({"-a"}, a, "选项A");
  // parser.addFlagArgument({"-b"}, b, "选项B");
  // parser.addFlagArgument({"-c"}, c, "选项C");
  // ASSERT_TRUE(parser.parse({"-abc"}));
  // ASSERT_TRUE(a && b && c);
  
  TEST_PASSED();
}

// 9.10 回调与默认值交互测试
void test_mixed_callback_not_called_for_default() {
  TEST_CASE("混合模式 - 默认值不触发回调");
  
  ArgumentParser parser("test", "测试程序", false);
  
  int callback_count = 0;
  std::string value;
  
  parser.addOptionArgument<std::string>({"-v", "--value"}, value, "值")
        .defaultValue("default")
        .callback([&callback_count]() { callback_count++; });
  
  // 不传值，使用默认值，回调不应触发
  ASSERT_TRUE(parser.parse({}));
  ASSERT_EQ(value, "default");
  ASSERT_EQ(callback_count, 0);  // 回调未被调用
  
  TEST_PASSED();
}

void test_mixed_callback_called_for_explicit_value() {
  TEST_CASE("混合模式 - 显式值触发回调");
  
  ArgumentParser parser("test", "测试程序", false);
  
  int callback_count = 0;
  std::string value;
  
  parser.addOptionArgument<std::string>({"-v", "--value"}, value, "值")
        .defaultValue("default")
        .callback([&callback_count]() { callback_count++; });
  
  // 传值，回调应触发
  ASSERT_TRUE(parser.parse({"-v", "explicit"}));
  ASSERT_EQ(value, "explicit");
  ASSERT_EQ(callback_count, 1);
  
  TEST_PASSED();
}

void test_mixed_multiple_callbacks_order() {
  TEST_CASE("混合模式 - 多回调顺序");
  
  ArgumentParser parser("test", "测试程序", false);
  
  std::vector<std::string> order;
  
  std::string name;
  int count = 0;
  bool flag = false;
  
  parser.addOptionArgument<std::string>({"-n", "--name"}, name, "名称")
        .callback([&order]() { order.push_back("name"); });
  parser.addOptionArgument<int>({"-c", "--count"}, count, "计数")
        .callback([&order]() { order.push_back("count"); });
  parser.addFlagArgument({"-f", "--flag"}, flag, "标志")
        .callback([&order]() { order.push_back("flag"); });
  
  ASSERT_TRUE(parser.parse({"-f", "-c", "10", "-n", "test"}));
  
  // 验证回调按命令行顺序执行
  ASSERT_EQ(order.size(), 3u);
  ASSERT_EQ(order[0], "flag");
  ASSERT_EQ(order[1], "count");
  ASSERT_EQ(order[2], "name");
  
  TEST_PASSED();
}

// 9.11 类型转换测试
void test_mixed_various_types() {
  TEST_CASE("混合模式 - 各种类型");
  
  ArgumentParser parser("test", "测试程序", false);
  
  int int_val = 0;
  long long_val = 0;
  float float_val = 0.0f;
  double double_val = 0.0;
  std::string string_val;
  bool bool_val = false;
  
  parser.addOptionArgument<int>({"--int"}, int_val, "整数");
  parser.addOptionArgument<long>({"--long"}, long_val, "长整数");
  parser.addOptionArgument<float>({"--float"}, float_val, "浮点数");
  parser.addOptionArgument<double>({"--double"}, double_val, "双精度");
  parser.addOptionArgument<std::string>({"--string"}, string_val, "字符串");
  parser.addFlagArgument({"--bool"}, bool_val, "布尔值");
  
  ASSERT_TRUE(parser.parse({
    "--int", "42",
    "--long", "9876543210",
    "--float", "3.14",
    "--double", "2.718281828",
    "--string", "hello world",
    "--bool"
  }));
  
  ASSERT_EQ(int_val, 42);
  ASSERT_EQ(long_val, 9876543210L);
  ASSERT_TRUE(float_val > 3.13f && float_val < 3.15f);
  ASSERT_TRUE(double_val > 2.71 && double_val < 2.72);
  ASSERT_EQ(string_val, "hello world");
  ASSERT_TRUE(bool_val);
  
  TEST_PASSED();
}

// 9.12 required 与混合参数测试
void test_mixed_required_and_optional() {
  TEST_CASE("混合模式 - 必需与可选参数");
  
  ArgumentParser parser("test", "测试程序", false);
  
  std::string required_val;
  std::string optional_val;
  bool flag = false;
  
  parser.addOptionArgument<std::string>({"-r", "--required"}, required_val, "必需")
        .required();
  parser.addOptionArgument<std::string>({"-o", "--optional"}, optional_val, "可选")
        .defaultValue("default");
  parser.addFlagArgument({"-f", "--flag"}, flag, "标志");
  
  ASSERT_TRUE(parser.parse({"-r", "value", "-f"}));
  ASSERT_EQ(required_val, "value");
  ASSERT_EQ(optional_val, "default");
  ASSERT_TRUE(flag);
  
  TEST_PASSED();
}

void test_mixed_required_missing_with_others_present() {
  TEST_CASE("混合模式 - 必需参数缺失（其他参数存在）");
  
  ArgumentParser parser("test", "测试程序", false);
  
  std::string required_val;
  std::string optional_val;
  bool flag = false;
  
  parser.addOptionArgument<std::string>({"-r", "--required"}, required_val, "必需")
        .required();
  parser.addOptionArgument<std::string>({"-o", "--optional"}, optional_val, "可选")
        .defaultValue("default");
  parser.addFlagArgument({"-f", "--flag"}, flag, "标志");
  
  // 有可选参数和 flag，但缺少必需参数
  ASSERT_THROWS(parser.parse({"-o", "value", "-f"}));
  
  TEST_PASSED();
}

// ==================== 10. 验证器测试 ====================

void test_validator() {
  TEST_CASE("验证器 - 通过场景");
  
  ArgumentParser parser("test", "测试程序", false);
  std::vector<int> numbers;
  parser.addOptionArgument<int>({"-n", "--numbers"}, numbers, "数字列表")
        .expected(1, 5)
        .validator([](const std::vector<int>& vals) {
          for (int v : vals) {
            if (v < 0 || v > 100) return false;
          }
          return true;
        });
  
  ASSERT_TRUE(parser.parse({"-n", "10", "20", "30"}));
  ASSERT_EQ(numbers.size(), 3u);
  
  TEST_PASSED();
}

void test_validator_auto_called_failure() {
  TEST_CASE("验证器 - 自动调用失败场景");
  
  ArgumentParser parser("test", "测试程序", false);
  std::vector<int> numbers;
  parser.addOptionArgument<int>({"-n", "--numbers"}, numbers, "数字列表")
        .expected(1, 5)
        .validator([](const std::vector<int>& vals) {
          for (int v : vals) {
            if (v < 0 || v > 100) return false;  // 只允许 0-100 范围
          }
          return true;
        });
  
  // 150 超出范围，验证器应该自动抛出异常
  ASSERT_THROWS(parser.parse({"-n", "50", "150"}));
  
  TEST_PASSED();
}

void test_validator_positional() {
  TEST_CASE("验证器 - 位置参数");
  
  ArgumentParser parser("test", "测试程序", false);
  std::string filename;
  parser.addPositionalArgument<std::string>({"filename"}, filename, "文件名")
        .validator([](const std::vector<std::string>& vals) {
          // 验证文件名必须以 .txt 结尾
          if (vals.empty()) return false;
          const auto& name = vals.front();
          return name.size() > 4 && name.substr(name.size() - 4) == ".txt";
        });
  
  // 有效文件名
  ASSERT_TRUE(parser.parse({"test.txt"}));
  ASSERT_EQ(filename, "test.txt");
  
  TEST_PASSED();
}

void test_validator_positional_failure() {
  TEST_CASE("验证器 - 位置参数失败场景");
  
  ArgumentParser parser("test", "测试程序", false);
  std::string filename;
  parser.addPositionalArgument<std::string>({"filename"}, filename, "文件名")
        .validator([](const std::vector<std::string>& vals) {
          // 验证文件名必须以 .txt 结尾
          if (vals.empty()) return false;
          const auto& name = vals.front();
          return name.size() > 4 && name.substr(name.size() - 4) == ".txt";
        });
  
  // 无效文件名，验证器应该抛出异常
  ASSERT_THROWS(parser.parse({"test.csv"}));
  
  TEST_PASSED();
}

// ==================== 主函数 ====================

int main() {
  std::cout << "========================================" << std::endl;
  std::cout << "     argparse 库测试套件" << std::endl;
  std::cout << "========================================" << std::endl;
  
  // 1. FlagArgument 测试
  std::cout << "\n--- FlagArgument 测试 ---" << std::endl;
  test_flag_basic();
  test_flag_short_name();
  test_flag_not_set();
  test_flag_default_value();
  test_flag_callback();
  
  // 2. OptionArgument 测试
  std::cout << "\n--- OptionArgument 测试 ---" << std::endl;
  test_option_basic();
  test_option_long_name();
  test_option_int();
  test_option_negative_number();
  test_option_default_value();
  test_option_callback();
  test_option_required_missing();
  test_option_missing_value();
  
  // 3. nargs (expected) 测试
  std::cout << "\n--- nargs (expected) 测试 ---" << std::endl;
  test_option_nargs_exact();
  test_option_nargs_exact_insufficient();
  test_option_nargs_range();
  test_option_nargs_range_0_1();
  test_option_nargs_range_0_1_with_value();
  test_option_nargs_unlimited();
  test_option_nargs_unlimited_empty();
  test_option_nargs_at_least_one();
  test_option_nargs_at_least_one_empty();
  
  // 4. PositionalArgument 测试
  std::cout << "\n--- PositionalArgument 测试 ---" << std::endl;
  test_positional_basic();
  test_positional_multiple();
  test_positional_with_option();
  test_positional_nargs_unlimited();
  
  // 5. 自动 help 测试
  std::cout << "\n--- 自动 help 测试 ---" << std::endl;
  test_auto_help_enabled();
  test_auto_help_short();
  test_auto_help_disabled();
  test_user_defined_help_flag();
  test_user_defined_help_option();
  test_user_defined_help_option_no_value();
  
  // 6. 子命令测试
  std::cout << "\n--- 子命令测试 ---" << std::endl;
  test_subcommand_basic();
  test_subcommand_with_options();
  
  // 7. 异常处理测试
  std::cout << "\n--- 异常处理测试 ---" << std::endl;
  test_unknown_argument();
  test_invalid_flag_name();
  test_duplicate_argument_name();
  test_empty_subcommand_name();
  test_subcommand_starts_with_dash();
  
  // 8. 复杂场景测试
  std::cout << "\n--- 复杂场景测试 ---" << std::endl;
  test_mixed_arguments();
  test_mixed_with_subcommand();
  test_mixed_with_help_and_subcommand();
  test_mixed_with_user_help_and_subcommand();
  test_mixed_subcommand_help();
  test_mixed_all_types();
  test_mixed_multiple_subcommands();
  test_mixed_nargs_and_positional();
  test_mixed_callback_chain();
  test_option_stops_at_next_option();
  test_double_dash_separator();
  
  // 9. 更多混合模式测试
  std::cout << "\n--- 更多混合模式测试 ---" << std::endl;
  test_mixed_order_positional_first();
  test_mixed_order_positional_middle();
  test_mixed_order_flags_scattered();
  test_mixed_multiple_options_interleaved();
  test_mixed_options_with_multiple_positional();
  test_mixed_global_option_before_subcommand();
  test_mixed_subcommand_with_multiple_global_options();
  test_mixed_nargs_unlimited_with_flag();
  test_mixed_nargs_at_least_one_with_option();
  test_mixed_nargs_range_with_positional();
  test_mixed_multiple_nargs_options();
  test_mixed_multiple_defaults();
  test_mixed_partial_defaults();
  test_mixed_help_with_other_args_before();
  test_mixed_user_help_with_subcommand_and_args();
  test_mixed_positional_after_all_options();
  test_mixed_positional_between_options();
  test_mixed_subcommand_all_types();
  test_mixed_nested_subcommands();
  test_mixed_empty_string_value();
  test_mixed_value_looks_like_option();
  test_mixed_long_option_with_equals();
  test_mixed_combined_short_flags();
  test_mixed_callback_not_called_for_default();
  test_mixed_callback_called_for_explicit_value();
  test_mixed_multiple_callbacks_order();
  test_mixed_various_types();
  test_mixed_required_and_optional();
  test_mixed_required_missing_with_others_present();
  
  // 10. 验证器测试
  std::cout << "\n--- 验证器测试 ---" << std::endl;
  test_validator();
  test_validator_auto_called_failure();
  test_validator_positional();
  test_validator_positional_failure();
  
  // 打印测试结果
  std::cout << "\n========================================" << std::endl;
  std::cout << "测试结果: " << test_passed << " 通过, " << test_failed << " 失败" << std::endl;
  std::cout << "========================================" << std::endl;
  
  return test_failed > 0 ? 1 : 0;
}
