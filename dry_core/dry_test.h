#pragma once

#include <cstdio>
#include <exception>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

namespace dry {

static int g_fail_count = 0;

// ====== 非致命断言（EXPECT_xxx，失败后继续） ======
#define DRY_EXPECT_EQ(a, b)                                          \
  do {                                                               \
    if (!((a) == (b))) {                                             \
      fprintf(stderr, "[FAIL] %s:%d: EXPECT_EQ failed\n",            \
              __FILE__, __LINE__);                                   \
      dry::g_fail_count++;                                           \
    }                                                                \
  } while (0)

#define DRY_EXPECT_NE(a, b)                                          \
  do {                                                               \
    if (!((a) != (b))) {                                             \
      fprintf(stderr, "[FAIL] %s:%d: EXPECT_NE failed\n",            \
              __FILE__, __LINE__);                                   \
      dry::g_fail_count++;                                           \
    }                                                                \
  } while (0)

#define DRY_EXPECT_TRUE(cond)                                        \
  do {                                                               \
    if (!(cond)) {                                                   \
      fprintf(stderr, "[FAIL] %s:%d: EXPECT_TRUE failed\n",          \
              __FILE__, __LINE__);                                   \
      dry::g_fail_count++;                                           \
    }                                                                \
  } while (0)

#define DRY_EXPECT_FALSE(cond)                                       \
  do {                                                               \
    if (cond) {                                                      \
      fprintf(stderr, "[FAIL] %s:%d: EXPECT_FALSE failed\n",         \
              __FILE__, __LINE__);                                   \
      dry::g_fail_count++;                                           \
    }                                                                \
  } while (0)

#define DRY_EXPECT_STREQ(a, b)                                       \
  do {                                                               \
    if (std::string(a) != std::string(b)) {                          \
      fprintf(stderr, "[FAIL] %s:%d: EXPECT_STREQ failed\n",         \
              __FILE__, __LINE__);                                   \
      dry::g_fail_count++;                                           \
    }                                                                \
  } while (0)

// ====== 致命断言（ASSERT_xxx，失败后抛异常退出当前测试） ======
#define DRY_ASSERT_EQ(a, b)                                          \
  do {                                                               \
    if (!((a) == (b))) {                                             \
      fprintf(stderr, "[FATAL] %s:%d: ASSERT_EQ failed\n",           \
              __FILE__, __LINE__);                                   \
      throw std::runtime_error("FATAL: ASSERT_EQ failed");           \
    }                                                                \
  } while (0)

#define DRY_ASSERT_TRUE(cond)                                        \
  do {                                                               \
    if (!(cond)) {                                                   \
      fprintf(stderr, "[FATAL] %s:%d: ASSERT_TRUE failed\n",         \
              __FILE__, __LINE__);                                   \
      throw std::runtime_error("FATAL: ASSERT_TRUE failed");         \
    }                                                                \
  } while (0)

// ====== 测试用例 ======
struct TestCase {
  std::string suite;
  std::string name;
  std::function<void()> func;
};

class TestRegistry {
 public:
  static TestRegistry& instance() {
    static TestRegistry reg;
    return reg;
  }

  void AddTest(const std::string& suite, const std::string& name,
               std::function<void()> func) {
    m_tests.push_back({suite, name, std::move(func)});
  }

  int RunAll() {
    int passed = 0;
    int total = 0;
    for (auto& test : m_tests) {
      g_fail_count = 0;
      fprintf(stderr, "[ RUN      ] %s.%s\n",
              test.suite.c_str(), test.name.c_str());
      try {
        test.func();
      } catch (const std::exception& e) {
        fprintf(stderr, "[EXCEPTION] %s.%s: %s\n",
                test.suite.c_str(), test.name.c_str(), e.what());
        g_fail_count++;
      }
      if (g_fail_count == 0) {
        passed++;
        fprintf(stderr, "[       OK ] %s.%s\n",
                test.suite.c_str(), test.name.c_str());
      } else {
        fprintf(stderr, "[  FAILED  ] %s.%s (%d failures)\n",
                test.suite.c_str(), test.name.c_str(), g_fail_count);
      }
      total++;
    }
    fprintf(stderr, "\n[  PASSED  ] %d / %d tests.\n", passed, total);
    return total - passed;
  }

 private:
  std::vector<TestCase> m_tests;
};

class Register {
 public:
  Register(const std::string& suite, const std::string& name,
           std::function<void()> func) {
    TestRegistry::instance().AddTest(suite, name, std::move(func));
  }
};

// ====== 测试用例宏 ======
#define DRY_TEST(suite, name)                                              \
  static void dry_test_##suite##_##name();                                \
  static ::dry::Register dry_test_reg_##suite##_##name(                   \
      #suite, #name, dry_test_##suite##_##name);                          \
  static void dry_test_##suite##_##name()

}  // namespace dry
