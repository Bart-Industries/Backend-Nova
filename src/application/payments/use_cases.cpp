#include "application/payments/use_cases.h"
#include "domain/common/errors.h"

namespace starcafe::application::payments
{
SearchOrdersForCashierUseCase::SearchOrdersForCashierUseCase(domain::IOrderRepository &orderRepository)
    : orderRepository_(orderRepository)
{
}

std::vector<domain::orders::Order> SearchOrdersForCashierUseCase::execute(const std::string &customerName,
                                                                           const std::string &tableNumber,
                                                                           const std::string &status)
{
    return orderRepository_.searchOrders(customerName, tableNumber, status);
}

PayOrderUseCase::PayOrderUseCase(domain::IOrderRepository &orderRepository, domain::IPaymentRepository &paymentRepository)
    : orderRepository_(orderRepository), paymentRepository_(paymentRepository)
{
}

domain::payments::Payment PayOrderUseCase::execute(const PayOrderCommand &command)
{
    const auto order = orderRepository_.findById(command.orderId);
    if (!order.has_value())
    {
        throw domain::DomainError("Order not found");
    }
    if (command.amount <= 0)
    {
        throw domain::DomainError("Paid amount must be greater than zero");
    }
    orderRepository_.updateStatus(command.orderId, domain::OrderStatus::PAID);
    return paymentRepository_.upsertPaid(command.orderId, command.amount);
}

ListPaymentsUseCase::ListPaymentsUseCase(domain::IPaymentRepository &paymentRepository) : paymentRepository_(paymentRepository) {}
std::vector<domain::payments::Payment> ListPaymentsUseCase::execute() { return paymentRepository_.listAll(); }
}  // namespace starcafe::application::payments
