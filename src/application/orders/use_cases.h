#pragma once

#include "application/common/dtos.h"
#include "domain/businesses/business.h"
#include "domain/repositories.h"

namespace starcafe::application::orders
{
struct PublicTableSession
{
    domain::businesses::Business business;
    domain::tables::RestaurantTable table;
    std::vector<domain::orders::Order> activeOrders;
    bool canCreateMoreOrders{true};
    std::int64_t activeOrdersCount{};
    std::int64_t remainingSlots{};
};

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
    GetOrderStatusForCustomerUseCase(domain::IOrderRepository &orderRepository,
                                     domain::IRestaurantTableRepository &tableRepository);
    domain::orders::Order execute(std::int64_t orderId, const std::string &qrToken);

  private:
    domain::IOrderRepository &orderRepository_;
    domain::IRestaurantTableRepository &tableRepository_;
};

class GetPublicTableSessionUseCase
{
  public:
    GetPublicTableSessionUseCase(domain::IRestaurantTableRepository &tableRepository,
                                 domain::IOrderRepository &orderRepository,
                                 domain::IBusinessRepository &businessRepository);
    PublicTableSession execute(const std::string &qrToken);

  private:
    domain::IRestaurantTableRepository &tableRepository_;
    domain::IOrderRepository &orderRepository_;
    domain::IBusinessRepository &businessRepository_;
};

class GetKitchenOrdersUseCase
{
  public:
    explicit GetKitchenOrdersUseCase(domain::IOrderRepository &orderRepository);
    std::vector<domain::orders::Order> execute(std::int64_t businessId);

  private:
    domain::IOrderRepository &orderRepository_;
};

class StartPreparingOrderUseCase
{
  public:
    explicit StartPreparingOrderUseCase(domain::IOrderRepository &orderRepository);
    void execute(std::int64_t businessId, std::int64_t orderId);

  private:
    domain::IOrderRepository &orderRepository_;
};

class MarkOrderItemReadyUseCase
{
  public:
    explicit MarkOrderItemReadyUseCase(domain::IOrderRepository &orderRepository);
    void execute(std::int64_t businessId, std::int64_t itemId);

  private:
    domain::IOrderRepository &orderRepository_;
};

class MarkOrderReadyUseCase
{
  public:
    explicit MarkOrderReadyUseCase(domain::IOrderRepository &orderRepository);
    void execute(std::int64_t businessId, std::int64_t orderId);

  private:
    domain::IOrderRepository &orderRepository_;
};

class GetOrdersHistoryUseCase
{
  public:
    explicit GetOrdersHistoryUseCase(domain::IOrderRepository &orderRepository);
    std::vector<domain::orders::Order> execute(std::int64_t businessId);

  private:
    domain::IOrderRepository &orderRepository_;
};

class CancelOrderUseCase
{
  public:
    explicit CancelOrderUseCase(domain::IOrderRepository &orderRepository);
    void execute(std::int64_t businessId, std::int64_t orderId);

  private:
    domain::IOrderRepository &orderRepository_;
};
}  // namespace starcafe::application::orders
