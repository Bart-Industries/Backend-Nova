#pragma once

#include "domain/identity/user.h"
#include "domain/menu/menu_models.h"
#include "domain/orders/order.h"
#include "domain/payments/payment.h"
#include "domain/tables/restaurant_table.h"

#include <optional>
#include <string>
#include <vector>

namespace starcafe::domain
{
class IUserRepository
{
  public:
    virtual ~IUserRepository() = default;
    virtual std::optional<identity::User> findByEmail(const std::string &email) = 0;
    virtual std::optional<identity::User> findById(std::int64_t id) = 0;
    virtual std::vector<identity::User> listActive() = 0;
    virtual std::int64_t countActiveAdmins() = 0;
    virtual identity::User create(const identity::User &user) = 0;
    virtual void deactivate(std::int64_t id) = 0;
};

class ICategoryRepository
{
  public:
    virtual ~ICategoryRepository() = default;
    virtual std::vector<menu::Category> listAll(bool onlyActive = true) = 0;
    virtual menu::Category create(const menu::Category &category) = 0;
    virtual menu::Category update(std::int64_t id, const menu::Category &category) = 0;
};

class IProductRepository
{
  public:
    virtual ~IProductRepository() = default;
    virtual std::optional<menu::Product> findById(std::int64_t id) = 0;
    virtual std::vector<menu::Product> listAll(bool onlyActive = true) = 0;
    virtual std::vector<menu::Product> listPublicMenu() = 0;
    virtual menu::Product create(const menu::Product &product) = 0;
    virtual menu::Product update(std::int64_t id, const menu::Product &product) = 0;
    virtual void markUnavailable(std::int64_t id) = 0;
    virtual void deactivate(std::int64_t id) = 0;
    virtual void assignAddon(std::int64_t productId, std::int64_t addonId) = 0;
};

class IAddonRepository
{
  public:
    virtual ~IAddonRepository() = default;
    virtual std::optional<menu::Addon> findById(std::int64_t id) = 0;
    virtual std::vector<menu::Addon> listAll(bool onlyActive = true) = 0;
    virtual menu::Addon create(const menu::Addon &addon) = 0;
    virtual menu::Addon update(std::int64_t id, const menu::Addon &addon) = 0;
};

class IRestaurantTableRepository
{
  public:
    virtual ~IRestaurantTableRepository() = default;
    virtual tables::RestaurantTable create(const tables::RestaurantTable &table) = 0;
    virtual std::vector<tables::RestaurantTable> listActive() = 0;
    virtual std::optional<tables::RestaurantTable> findByQrToken(const std::string &qrToken) = 0;
    virtual std::optional<tables::RestaurantTable> findById(std::int64_t id) = 0;
    virtual tables::RestaurantTable updateQrToken(std::int64_t id, const std::string &qrToken) = 0;
    virtual void deactivate(std::int64_t id) = 0;
};

class IOrderRepository
{
  public:
    virtual ~IOrderRepository() = default;
    virtual orders::Order create(const orders::Order &order) = 0;
    virtual std::optional<orders::Order> findById(std::int64_t id) = 0;
    virtual std::vector<orders::Order> listKitchenActive() = 0;
    virtual std::vector<orders::Order> listHistory() = 0;
    virtual std::vector<orders::Order> searchOrders(const std::string &customerName,
                                                    const std::string &tableNumber,
                                                    const std::string &status) = 0;
    virtual void updateStatus(std::int64_t orderId, OrderStatus status) = 0;
    virtual void updateOrderItemStatus(std::int64_t itemId, OrderItemStatus status) = 0;
    virtual bool allItemsReady(std::int64_t orderId) = 0;
};

class IPaymentRepository
{
  public:
    virtual ~IPaymentRepository() = default;
    virtual std::optional<payments::Payment> findByOrderId(std::int64_t orderId) = 0;
    virtual payments::Payment upsertPaid(std::int64_t orderId, double amount) = 0;
    virtual std::vector<payments::Payment> listAll() = 0;
};
}  // namespace starcafe::domain
