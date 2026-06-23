#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace starcafe::domain::menu
{
struct Category
{
    std::int64_t id{};
    std::string name;
    std::string description;
    bool isActive{true};
};

struct Addon
{
    std::int64_t id{};
    std::string name;
    double price{};
    bool isActive{true};
};

struct Product
{
    std::int64_t id{};
    std::int64_t categoryId{};
    std::string name;
    std::string description;
    double price{};
    bool isAvailable{true};
    bool isActive{true};
    std::vector<Addon> addons;
};
}  // namespace starcafe::domain::menu
