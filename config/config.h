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
    bool init_config(const std::string &path);
    bool exist(const std::string &section, const std::string &key);
    bool exist(const std::string &section);
    bool get_value(const std::string &section, const std::string &key, std::string &val);
    bool get_value(const std::string &section, const std::string &key, int &val);
    bool get_value(const std::string &section, const std::string &key, double &val);

    void get_value(const std::string& section, const std::string& key, std::string& val, const std::string& default_value);
    void get_value(const std::string &section, const std::string &key, int &val, const int &default_value);
    void get_value(const std::string &section, const std::string &key, double &val, const double &default_value);
    std::string get_string(const std::string &section, const std::string &key, const std::string &default_value = "");
    int get_int(const std::string &section, const std::string &key, const int &default_value = 0);
    double get_double(const std::string &section, const std::string &key, const double &default_value = 0.0);
    std::unordered_map<std::string, Section> get_sections();
    std::vector<std::string> get_section_names();
    Section get_section_kvs(const std::string& section);

private:
    std::unordered_map<std::string, Section> m_sections;
};
};

#endif