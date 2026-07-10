#pragma once

#include "domain/common/enums.h"

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>

namespace starcafe::domain::identity
{
struct User
{
    std::int64_t id{};
    std::string name;
    std::string email;
    std::string passwordHash;
    std::optional<std::int64_t> businessId;
    UserRole role{UserRole::KITCHEN};
    bool isActive{true};
    std::chrono::system_clock::time_point createdAt{};
};
}  // namespace starcafe::domain::identity
