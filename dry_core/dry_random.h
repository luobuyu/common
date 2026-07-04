#ifndef DRY_RANDOM_H
#define DRY_RANDOM_H
#include <random>
#include <vector>

#include "dry_time.h"
namespace dry {
class Random {
 public:
  static std::default_random_engine &GetRandomEngine() {
    static std::default_random_engine gen(
        static_cast<unsigned>(std::chrono::steady_clock::now().time_since_epoch().count()));
    return gen;
  }
  static void SetSeed(unsigned int seed) { GetRandomEngine().seed(seed); }
  // [lb, ub)
  static int Rand(int lb, int ub, std::default_random_engine &gen = GetRandomEngine()) {
    return std::uniform_int_distribution<int>(lb, ub - 1)(gen);
  }
  // [0, ub)
  static int Rand(int ub, std::default_random_engine &gen = GetRandomEngine()) {
    return std::uniform_int_distribution<int>(0, ub - 1)(gen);
  }
  // [0, 1)
  static double Rand(std::default_random_engine &gen = GetRandomEngine()) {
    return std::uniform_real_distribution<double>(0, 1)(gen);
  }
  // [lb, ub)
  static double Rand(double lb, double ub, std::default_random_engine &gen = GetRandomEngine()) {
    return std::uniform_real_distribution<double>(lb, ub)(gen);
  }
  // [0, weight.size() - 1) 返回的是数组下标
  static int Rand(const std::vector<int> &weights,
                  std::default_random_engine &gen = GetRandomEngine()) {
    return std::discrete_distribution<>(weights.begin(), weights.end())(gen);
  }
  // 传入迭代器 [begin, end)
  template <typename Iterator>
  static int Rand(const Iterator &begin, const Iterator &end,
                  std::default_random_engine &gen = GetRandomEngine()) {
    return std::discrete_distribution<>(begin, end)(gen);
  }
};
};  // namespace dry

#endif