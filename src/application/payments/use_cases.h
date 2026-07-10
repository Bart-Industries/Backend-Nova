#pragma once

#include "application/common/dtos.h"
#include "domain/repositories.h"

namespace starcafe::application::payments
{
class SearchOrdersForCashierUseCase
{
  public:
    explicit SearchOrdersForCashierUseCase(domain::IOrderRepository &orderRepository);
    std::vector<domain::orders::Order> execute(std::int64_t businessId,
                                               const std::string &customerName,
                                               const std::string &tableNumber,
                                               const std::string &status);

  private:
    domain::IOrderRepository &orderRepository_;
};

class PayOrderUseCase
{
  public:
    PayOrderUseCase(domain::IOrderRepository &orderRepository, domain::IPaymentRepository &paymentRepository);
    domain::payments::Payment execute(const PayOrderCommand &command);

  private:
    domain::IOrderRepository &orderRepository_;
    domain::IPaymentRepository &paymentRepository_;
};

class ListPaymentsUseCase
{
  public:
    explicit ListPaymentsUseCase(domain::IPaymentRepository &paymentRepository);
    std::vector<domain::payments::Payment> execute(std::int64_t businessId);

  private:
    domain::IPaymentRepository &paymentRepository_;
};
}  // namespace starcafe::application::payments
