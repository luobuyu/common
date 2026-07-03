#ifndef CONFIG_CONFIG_H
#define CONFIG_CONFIG_H
#include <string>
#include <unordered_map>
#include <vector>
namespace dry {
namespace config {

class Config {
 public:
  using Section = std::unordered_map<std::string, std::string>;
  Config() = default;
  ~Config() = default;
  bool InitConfig(const std::string &path);
  bool Exist(const std::string &section, const std::string &key);
  bool Exist(const std::string &section);
  bool GetValue(const std::string &section, const std::string &key,
                std::string &val);
  bool GetValue(const std::string &section, const std::string &key, int &val);
  bool GetValue(const std::string &section, const std::string &key,
                double &val);

  void GetValue(const std::string &section, const std::string &key,
                std::string &val, const std::string &default_value);
  void GetValue(const std::string &section, const std::string &key, int &val,
                const int &default_value);
  void GetValue(const std::string &section, const std::string &key, double &val,
                const double &default_value);
  std::string GetString(const std::string &section, const std::string &key,
                        const std::string &default_value = "");
  int GetInt(const std::string &section, const std::string &key,
             const int &default_value = 0);
  double GetDouble(const std::string &section, const std::string &key,
                   const double &default_value = 0.0);
  std::unordered_map<std::string, Section> GetSections();
  std::vector<std::string> GetSectionNames();
  Section GetSectionKVs(const std::string &section);
  void SetValue(const std::string &section, const std::string &key,
                const std::string &val);

 private:
  std::unordered_map<std::string, Section> m_sections;
};
};  // namespace config
}  // namespace dry

#endif