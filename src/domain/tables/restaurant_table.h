#pragma once

#include <cstdint>
#include <string>

namespace starcafe::domain::tables
{
struct RestaurantTable
{
    std::int64_t id{};
    std::int64_t businessId{};
    std::string businessName;
    std::string businessSlug;
    int tableNumber{};
    std::string qrToken;
    bool isActive{true};
};
}  // namespace starcafe::domain::tables
