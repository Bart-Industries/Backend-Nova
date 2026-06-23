#pragma once

#include <stdexcept>
#include <string>

namespace starcafe::domain
{
class DomainError : public std::runtime_error
{
  public:
    explicit DomainError(const std::string &message) : std::runtime_error(message) {}
};
}  // namespace starcafe::domain
