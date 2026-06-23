#pragma once

#include "application/common/dtos.h"
#include "domain/repositories.h"

namespace starcafe::application::orders
{
class CreateOrderFromTableUseCase
{
  public:
    CreateOrderFromTableUseCase(domain::IRestaurantTableRepository &tableRepository,
                                domain::IProductRepository &productRepository,
                                domain::IAddonRepository &addonRepository,
                                domain::IOrderRepository &orderRepository);
    domain::orders::Order execute(const CreateOrderFromTableCommand &command);

  private:
    domain::IRestaurantTableRepository &tableRepository_;
    domain::IProductRepository &productRepository_;
    domain::IAddonRepository &addonRepository_;
    domain::IOrderRepository &orderRepository_;
};

class GetOrderStatusForCustomerUseCase
{
  public:
    explicit GetOrderStatusForCustomerUseCase(domain::IOrderRepository &orderRepository);
    domain::orders::Order execute(std::int64_t orderId);

  private:
    domain::IOrderRepository &orderRepository_;
};

class GetKitchenOrdersUseCase
{
  public:
    explicit GetKitchenOrdersUseCase(domain::IOrderRepository &orderRepository);
    std::vector<domain::orders::Order> execute();

  private:
    domain::IOrderRepository &orderRepository_;
};

class StartPreparingOrderUseCase
{
  public:
    explicit StartPreparingOrderUseCase(domain::IOrderRepository &orderRepository);
    void execute(std::int64_t orderId);

  private:
    domain::IOrderRepository &orderRepository_;
};

class MarkOrderItemReadyUseCase
{
  public:
    explicit MarkOrderItemReadyUseCase(domain::IOrderRepository &orderRepository);
    void execute(std::int64_t itemId);

  private:
    domain::IOrderRepository &orderRepository_;
};

class MarkOrderReadyUseCase
{
  public:
    explicit MarkOrderReadyUseCase(domain::IOrderRepository &orderRepository);
    void execute(std::int64_t orderId);

  private:
    domain::IOrderRepository &orderRepository_;
};

class GetOrdersHistoryUseCase
{
  public:
    explicit GetOrdersHistoryUseCase(domain::IOrderRepository &orderRepository);
    std::vector<domain::orders::Order> execute();

  private:
    domain::IOrderRepository &orderRepository_;
};

class CancelOrderUseCase
{
  public:
    explicit CancelOrderUseCase(domain::IOrderRepository &orderRepository);
    void execute(std::int64_t orderId);

  private:
    domain::IOrderRepository &orderRepository_;
};
}  // namespace starcafe::application::orders
