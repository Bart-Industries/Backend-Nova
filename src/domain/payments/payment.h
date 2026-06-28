#pragma once

#include "domain/common/enums.h"

#include <cstdint>
#include <string>

namespace starcafe::domain::payments
{
struct Payment
{
    std::int64_t id{};
    std::int64_t businessId{};
    std::int64_t orderId{};
    double amount{};
    PaymentStatus status{PaymentStatus::PENDING};
    std::string paidAt;
};
}  // namespace starcafe::domain::payments
