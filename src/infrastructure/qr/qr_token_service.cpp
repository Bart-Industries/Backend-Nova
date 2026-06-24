#include "infrastructure/qr/qr_token_service.h"

#include <drogon/utils/Utilities.h>

namespace starcafe::infrastructure::qr
{
std::string QrTokenService::generateToken() const
{
    return drogon::utils::getUuid();
}
}  // namespace starcafe::infrastructure::qr
