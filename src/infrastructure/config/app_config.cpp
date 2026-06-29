#include "infrastructure/config/app_config.h"
#include "infrastructure/config/env_loader.h"

#include <algorithm>
#include <sstream>

namespace starcafe::infrastructure::config
{
AppConfig AppConfig::load()
{
    EnvLoader::loadFromFile(".env");

    AppConfig config;
    config.appEnv = EnvLoader::getOptional("APP_ENV", "local");
    config.appPort = static_cast<std::uint16_t>(std::stoi(EnvLoader::getRequired("APP_PORT")));
    config.databaseUrl = EnvLoader::getRequired("DATABASE_URL");
    config.jwtSecret = EnvLoader::getRequired("JWT_SECRET");
    config.jwtExpiresIn = std::stoll(EnvLoader::getRequired("JWT_EXPIRES_IN"));
    config.frontendBaseUrl = EnvLoader::getRequired("FRONTEND_BASE_URL");
    config.bcryptCost = std::stoi(EnvLoader::getOptional("BCRYPT_COST", "12"));
    config.appThreads = std::max(1, std::stoi(EnvLoader::getOptional("APP_THREADS", "4")));
    config.cloudinaryCloudName = EnvLoader::getRequired("CLOUDINARY_CLOUD_NAME");
    config.cloudinaryApiKey = EnvLoader::getRequired("CLOUDINARY_API_KEY");
    config.cloudinaryApiSecret = EnvLoader::getRequired("CLOUDINARY_API_SECRET");
    config.cloudinaryFolder = EnvLoader::getOptional("CLOUDINARY_FOLDER", "nova/products");
    config.maxProductImageSizeMb = std::stoll(EnvLoader::getOptional("MAX_PRODUCT_IMAGE_SIZE_MB", "5"));

    std::stringstream stream(EnvLoader::getOptional("CORS_ALLOWED_ORIGINS"));
    std::string origin;
    while (std::getline(stream, origin, ','))
    {
        if (!origin.empty())
        {
            config.corsAllowedOrigins.push_back(origin);
        }
    }

    return config;
}
}  // namespace starcafe::infrastructure::config
