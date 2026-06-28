#include "application/payments/use_cases.h"
#include "domain/common/errors.h"

#include <algorithm>
#include <cctype>

namespace starcafe::application::payments
{
namespace
{
std::string normalizeStatus(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    return value;
}
}  // namespace

SearchOrdersForCashierUseCase::SearchOrdersForCashierUseCase(domain::IOrderRepository &orderRepository)
    : orderRepository_(orderRepository)
{
}

std::vector<domain::orders::Order> SearchOrdersForCashierUseCase::execute(std::int64_t businessId,
                                                                           const std::string &customerName,
                                                                           const std::string &tableNumber,
                                                                           const std::string &status)
{
    std::string normalizedStatus = status;
    if (!status.empty())
    {
        try
        {
            normalizedStatus = domain::toString(domain::orderStatusFromString(normalizeStatus(status)));
        }
        catch (const std::invalid_argument &)
        {
            throw domain::DomainError("Invalid order status");
        }
    }
    return orderRepository_.searchOrders(businessId, customerName, tableNumber, normalizedStatus);
}

PayOrderUseCase::PayOrderUseCase(domain::IOrderRepository &orderRepository, domain::IPaymentRepository &paymentRepository)
    : orderRepository_(orderRepository), paymentRepository_(paymentRepository)
{
}

domain::payments::Payment PayOrderUseCase::execute(const PayOrderCommand &command)
{
    const auto order = orderRepository_.findById(command.orderId);
    if (!order.has_value() || order->businessId != command.businessId)
    {
        throw domain::DomainError("Order not found");
    }
    if (command.amount <= 0)
    {
        throw domain::DomainError("Paid amount must be greater than zero");
    }
    orderRepository_.updateStatus(command.orderId, domain::OrderStatus::PAID);
    return paymentRepository_.upsertPaid(command.businessId, command.orderId, command.amount);
}

ListPaymentsUseCase::ListPaymentsUseCase(domain::IPaymentRepository &paymentRepository) : paymentRepository_(paymentRepository) {}
std::vector<domain::payments::Payment> ListPaymentsUseCase::execute(std::int64_t businessId) { return paymentRepository_.listAll(businessId); }
}  // namespace starcafe::application::payments
