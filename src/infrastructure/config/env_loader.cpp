#include "infrastructure/config/env_loader.h"

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace starcafe::infrastructure::config
{
std::unordered_map<std::string, std::string> EnvLoader::cache_{};

void EnvLoader::loadFromFile(const std::string &path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty() || line.starts_with('#'))
        {
            continue;
        }

        const auto separator = line.find('=');
        if (separator == std::string::npos)
        {
            continue;
        }

        const auto key = line.substr(0, separator);
        const auto value = line.substr(separator + 1);
        cache_[key] = value;
#ifdef _WIN32
        _putenv_s(key.c_str(), value.c_str());
#else
        setenv(key.c_str(), value.c_str(), 1);
#endif
    }
}

std::string EnvLoader::getRequired(const std::string &key)
{
    if (const char *value = std::getenv(key.c_str()))
    {
        return value;
    }
    if (cache_.contains(key))
    {
        return cache_.at(key);
    }
    throw std::runtime_error("Missing required env var: " + key);
}

std::string EnvLoader::getOptional(const std::string &key, const std::string &defaultValue)
{
    if (const char *value = std::getenv(key.c_str()))
    {
        return value;
    }
    if (cache_.contains(key))
    {
        return cache_.at(key);
    }
    return defaultValue;
}
}  // namespace starcafe::infrastructure::config
