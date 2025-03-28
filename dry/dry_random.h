#ifndef DRY_RANDOM_H
#define DRY_RANDOM_H
#include <random>
#include <vector>
#include "dry_time.h"
namespace dry
{
    class Random
    {
    public:
        static std::default_random_engine& getRandomEngine()
        {
            static std::default_random_engine gen(static_cast<unsigned>(dry::clock::now().time_since_epoch().count()));
            return gen;
        }
        static void setSeed(unsigned int seed) {
            getRandomEngine().seed(seed);
        }
        // [lb, ub)
        static int rand(int lb, int ub, std::default_random_engine &gen = getRandomEngine())
        {
            return std::uniform_int_distribution<int>(lb, ub - 1)(gen);
        }
        // [0, ub)
        static int rand(int ub, std::default_random_engine &gen = getRandomEngine())
        {
            return std::uniform_int_distribution<int>(0, ub - 1)(gen);
        }
        // [0, 1)
        static double rand(std::default_random_engine &gen = getRandomEngine())
        {
            return std::uniform_real_distribution<double>(0, 1)(gen);
        }
        // [lb, ub)
        static double rand(double lb, double ub, std::default_random_engine &gen = getRandomEngine())
        {
            return std::uniform_real_distribution<double>(lb, ub)(gen);
        }
        // [0, weight.size() - 1) 返回的是数组下标
        static int rand(const std::vector<int> &weights, std::default_random_engine &gen = getRandomEngine())
        {
            return std::discrete_distribution<>(weights.begin(), weights.end())(gen);
        }
        // 传入迭代器 [begin, end) 
        template<typename Iterator>
        static int rand(const Iterator& begin, const Iterator& end, std::default_random_engine &gen = getRandomEngine())
        {
            return std::discrete_distribution<>(begin, end)(gen);
        }
    };
};

#endif