#pragma once

#include "domain/businesses/business.h"
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
class IBusinessRepository
{
  public:
    virtual ~IBusinessRepository() = default;
    virtual std::optional<businesses::Business> findById(std::int64_t id) = 0;
    virtual std::optional<businesses::Business> findBySlug(const std::string &slug) = 0;
    virtual std::optional<businesses::Business> findSingleActive() = 0;
    virtual std::vector<businesses::Business> listAll(bool onlyActive = false) = 0;
    virtual businesses::Business create(const businesses::Business &business) = 0;
};

class IUserRepository
{
  public:
    virtual ~IUserRepository() = default;
    virtual std::optional<identity::User> findByEmail(const std::string &email) = 0;
    virtual std::optional<identity::User> findById(std::int64_t id) = 0;
    virtual std::vector<identity::User> listActive(std::optional<std::int64_t> businessId = std::nullopt) = 0;
    virtual std::int64_t countActiveByRole(UserRole role, std::optional<std::int64_t> businessId = std::nullopt) = 0;
    virtual identity::User create(const identity::User &user) = 0;
    virtual void deactivate(std::int64_t id) = 0;
};

class ICategoryRepository
{
  public:
    virtual ~ICategoryRepository() = default;
    virtual std::vector<menu::Category> listAll(std::int64_t businessId, bool onlyActive = true) = 0;
    virtual menu::Category create(const menu::Category &category) = 0;
    virtual menu::Category update(std::int64_t id, std::int64_t businessId, const menu::Category &category) = 0;
};

class IProductRepository
{
  public:
    virtual ~IProductRepository() = default;
    virtual std::optional<menu::Product> findById(std::int64_t id) = 0;
    virtual std::vector<menu::Product> listAll(std::int64_t businessId, bool onlyActive = true) = 0;
    virtual std::vector<menu::Product> listPublicMenu(std::int64_t businessId) = 0;
    virtual menu::Product create(const menu::Product &product) = 0;
    virtual menu::Product update(std::int64_t id, const menu::Product &product) = 0;
    virtual void activate(std::int64_t id) = 0;
    virtual void markUnavailable(std::int64_t id) = 0;
    virtual void deactivate(std::int64_t id) = 0;
    virtual void assignAddon(std::int64_t productId, std::int64_t addonId) = 0;
};

class IProductImageRepository
{
  public:
    virtual ~IProductImageRepository() = default;
    virtual std::optional<menu::ProductImage> findMainByProductId(std::int64_t productId) = 0;
    virtual menu::ProductImage upsertMain(const menu::ProductImage &image) = 0;
    virtual void deleteMain(std::int64_t productId) = 0;
};

class IAddonRepository
{
  public:
    virtual ~IAddonRepository() = default;
    virtual std::optional<menu::Addon> findById(std::int64_t id) = 0;
    virtual std::vector<menu::Addon> listAll(std::int64_t businessId, bool onlyActive = true) = 0;
    virtual menu::Addon create(const menu::Addon &addon) = 0;
    virtual menu::Addon update(std::int64_t id, std::int64_t businessId, const menu::Addon &addon) = 0;
};

class IRestaurantTableRepository
{
  public:
    virtual ~IRestaurantTableRepository() = default;
    virtual tables::RestaurantTable create(const tables::RestaurantTable &table) = 0;
    virtual std::vector<tables::RestaurantTable> listActive(std::int64_t businessId) = 0;
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
    virtual std::vector<orders::Order> listKitchenActive(std::int64_t businessId) = 0;
    virtual std::vector<orders::Order> listHistory(std::int64_t businessId) = 0;
    virtual std::vector<orders::Order> listActiveByTableId(std::int64_t tableId) = 0;
    virtual std::int64_t countActiveByTableId(std::int64_t tableId) = 0;
    virtual std::vector<orders::Order> searchOrders(std::int64_t businessId,
                                                    const std::string &customerName,
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
    virtual payments::Payment upsertPaid(std::int64_t businessId, std::int64_t orderId, double amount) = 0;
    virtual std::vector<payments::Payment> listAll(std::int64_t businessId) = 0;
};
}  // namespace starcafe::domain
