#pragma once

#include <string>

namespace starcafe::infrastructure::qr
{
class QrTokenService
{
  public:
    std::string generateToken() const;
};
}  // namespace starcafe::infrastructure::qr
