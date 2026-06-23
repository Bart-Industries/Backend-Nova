#pragma once

#include <cstdint>
#include <string>

namespace starcafe::domain::tables
{
struct RestaurantTable
{
    std::int64_t id{};
    int tableNumber{};
    std::string qrToken;
    bool isActive{true};
};
}  // namespace starcafe::domain::tables
