#pragma once

#include <cstdint>
#include <string>

namespace starcafe::domain::businesses
{
struct Business
{
    std::int64_t id{};
    std::string name;
    std::string slug;
    std::string logoUrl;
    std::string primaryColor;
    bool isActive{true};
    std::string createdAt;
};
}  // namespace starcafe::domain::businesses
