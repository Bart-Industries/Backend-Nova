#pragma once

#include <string>
#include <unordered_map>

namespace starcafe::infrastructure::config
{
class EnvLoader
{
  public:
    static void loadFromFile(const std::string &path);
    static std::string getRequired(const std::string &key);
    static std::string getOptional(const std::string &key, const std::string &defaultValue = "");

  private:
    static std::unordered_map<std::string, std::string> cache_;
};
}  // namespace starcafe::infrastructure::config
