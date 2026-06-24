#pragma once

#include <string>

namespace starcafe::infrastructure::security
{
class PasswordHasher
{
  public:
    explicit PasswordHasher(int cost);
    std::string hash(const std::string &plainText) const;
    bool verify(const std::string &plainText, const std::string &encoded) const;

  private:
    int cost_;
};
}  // namespace starcafe::infrastructure::security
