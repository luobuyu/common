#ifndef CONFIG_CONFIG_H
#define CONFIG_CONFIG_H
#include <string>
#include <vector>
#include <unordered_map>
namespace config
{

class Config
{
public:
    using Section = std::unordered_map<std::string, std::string>;
    Config() = default;
    ~Config() = default;
    bool initConfig(const std::string &path);
    bool exist(const std::string &section, const std::string &key);
    bool exist(const std::string &section);
    bool getValue(const std::string &section, const std::string &key, std::string &val);
    bool getValue(const std::string &section, const std::string &key, int &val);
    bool getValue(const std::string &section, const std::string &key, double &val);

    void getValue(const std::string& section, const std::string& key, std::string& val, const std::string& default_value);
    void getValue(const std::string &section, const std::string &key, int &val, const int &default_value);
    void getValue(const std::string &section, const std::string &key, double &val, const double &default_value);
    std::string getString(const std::string &section, const std::string &key, const std::string &default_value = "");
    int getInt(const std::string &section, const std::string &key, const int &default_value = 0);
    double getDouble(const std::string &section, const std::string &key, const double &default_value = 0.0);
    std::unordered_map<std::string, Section> getSections();
    std::vector<std::string> getSectionNames();
    Section getSectionKVs(const std::string& section);
    void setValue(const std::string &section, const std::string &key, const std::string &val);

private:
    std::unordered_map<std::string, Section> m_sections;
};
};

#endif