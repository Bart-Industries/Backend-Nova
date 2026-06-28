#pragma once

#include "domain/common/enums.h"

#include <cstdint>
#include <optional>
#include <string>

namespace starcafe::infrastructure::security
{
struct JwtClaims
{
    std::int64_t userId{};
    std::optional<std::int64_t> businessId;
    std::string name;
    std::string email;
    std::string role;
    std::int64_t exp{};
};

class JwtService
{
  public:
    JwtService(std::string secret, std::int64_t expiresInSeconds);
    std::string createToken(std::int64_t userId,
                            std::optional<std::int64_t> businessId,
                            const std::string &name,
                            const std::string &email,
                            domain::UserRole role) const;
    std::optional<JwtClaims> verify(const std::string &token) const;

  private:
    std::string secret_;
    std::int64_t expiresInSeconds_;
};
}  // namespace starcafe::infrastructure::security
