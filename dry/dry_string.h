#ifndef DRY_STRING_H
#define DRY_STRING_H
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
namespace dry
{

    // split
    static std::vector<std::string> stringSplit(const std::string &s, const std::string &delim)
    {
        std::vector<std::string> ret;
        size_t start = 0, end = 0;
        size_t len = s.length();
        // 逐个找到子串并分割
        while ((end = s.find(delim, start)) != std::string::npos)
        {
            std::string substr = s.substr(start, end - start);
            if (!substr.empty())
                ret.emplace_back(substr); // 提取子串
            start = end + delim.length(); // 跳过分隔符
        }
        // 处理最后一个部分
        std::string substr = s.substr(start);
        if (!substr.empty())
            ret.emplace_back(substr);
        return ret;
    }
    // split
    static std::vector<std::string> stringSplit(const std::string& str, char delim = ' ') {
        return stringSplit(str, std::string(1, delim));
    }

    static std::string stringTrim(const std::string &str)
    {
        size_t first = str.find_first_not_of(" \t");
        if (first == std::string::npos)
            return "";
        size_t last = str.find_last_not_of(" \t");
        return str.substr(first, last - first + 1);
    }
};

#endif