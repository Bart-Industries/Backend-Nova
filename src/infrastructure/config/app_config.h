#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace starcafe::infrastructure::config
{
struct AppConfig
{
    std::string appEnv;
    std::uint16_t appPort{};
    std::string databaseUrl;
    std::string jwtSecret;
    std::int64_t jwtExpiresIn{};
    std::vector<std::string> corsAllowedOrigins;
    std::string frontendBaseUrl;
    int bcryptCost{};
    std::string uploadDir;
    std::int64_t maxProductImageSizeMb{};
    std::string publicFilesBaseUrl;

    static AppConfig load();
};
}  // namespace starcafe::infrastructure::config
