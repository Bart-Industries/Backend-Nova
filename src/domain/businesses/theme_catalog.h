#pragma once

#include "domain/common/errors.h"

#include <array>
#include <optional>
#include <string>
#include <string_view>

namespace starcafe::domain::businesses
{
struct ThemeOption
{
    std::string_view key;
    std::string_view primaryColor;
};

inline constexpr std::array<ThemeOption, 15> kThemeCatalog{{
    {"forest", "#1f4b3f"},
    {"copper", "#7a4b2a"},
    {"espresso", "#4a2c24"},
    {"sunset", "#c75b39"},
    {"terracotta", "#a74d3c"},
    {"olive", "#5d6b2d"},
    {"ocean", "#1d5f7a"},
    {"midnight", "#24324a"},
    {"plum", "#5a3d5c"},
    {"berry", "#8c2f4f"},
    {"gold", "#b88628"},
    {"sand", "#b79b6c"},
    {"slate", "#4c5a67"},
    {"coral", "#d96c5f"},
    {"mint", "#3f8f7b"},
}};

inline const ThemeOption &themeByKey(const std::string &key)
{
    for (const auto &theme : kThemeCatalog)
    {
        if (theme.key == key)
        {
            return theme;
        }
    }
    throw domain::DomainError("Invalid themeKey. Use one of the predefined business themes");
}

inline std::optional<std::string_view> themeKeyFromColor(const std::string &primaryColor)
{
    for (const auto &theme : kThemeCatalog)
    {
        if (theme.primaryColor == primaryColor)
        {
            return theme.key;
        }
    }
    return std::nullopt;
}
}  // namespace starcafe::domain::businesses
