#pragma once

#include <string>

namespace starcafe::application
{
template <typename T> struct ApiResult
{
    bool success{true};
    std::string message;
    T data{};
};
}  // namespace starcafe::application
