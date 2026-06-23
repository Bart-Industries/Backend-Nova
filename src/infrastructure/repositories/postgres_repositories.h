#pragma once

#include "domain/repositories.h"

#include <drogon/orm/DbClient.h>

namespace starcafe::infrastructure::repositories
{
class PostgresUserRepository : public domain::IUserRepository
{
  public:
    explicit PostgresUserRepository(drogon::orm::DbClientPtr db);
    std::optional<domain::identity::User> findByEmail(const std::string &email) override;
    std::optional<domain::identity::User> findById(std::int64_t id) override;
    std::vector<domain::identity::User> listActive() override;
    std::int64_t countActiveAdmins() override;
    domain::identity::User create(const domain::identity::User &user) override;
    void deactivate(std::int64_t id) override;

  private:
    drogon::orm::DbClientPtr db_;
};

class PostgresCategoryRepository : public domain::ICategoryRepository
{
  public:
    explicit PostgresCategoryRepository(drogon::orm::DbClientPtr db);
    std::vector<domain::menu::Category> listAll(bool onlyActive = true) override;
    domain::menu::Category create(const domain::menu::Category &category) override;
    domain::menu::Category update(std::int64_t id, const domain::menu::Category &category) override;

  private:
    drogon::orm::DbClientPtr db_;
};

class PostgresAddonRepository : public domain::IAddonRepository
{
  public:
    explicit PostgresAddonRepository(drogon::orm::DbClientPtr db);
    std::optional<domain::menu::Addon> findById(std::int64_t id) override;
    std::vector<domain::menu::Addon> listAll(bool onlyActive = true) override;
    domain::menu::Addon create(const domain::menu::Addon &addon) override;
    domain::menu::Addon update(std::int64_t id, const domain::menu::Addon &addon) override;

  private:
    drogon::orm::DbClientPtr db_;
};

class PostgresProductRepository : public domain::IProductRepository
{
  public:
    PostgresProductRepository(drogon::orm::DbClientPtr db, domain::IAddonRepository &addonRepository);
    std::optional<domain::menu::Product> findById(std::int64_t id) override;
    std::vector<domain::menu::Product> listAll(bool onlyActive = true) override;
    std::vector<domain::menu::Product> listPublicMenu() override;
    domain::menu::Product create(const domain::menu::Product &product) override;
    domain::menu::Product update(std::int64_t id, const domain::menu::Product &product) override;
    void markUnavailable(std::int64_t id) override;
    void deactivate(std::int64_t id) override;
    void assignAddon(std::int64_t productId, std::int64_t addonId) override;

  private:
    std::vector<domain::menu::Addon> loadAddons(std::int64_t productId);
    drogon::orm::DbClientPtr db_;
    domain::IAddonRepository &addonRepository_;
};

class PostgresRestaurantTableRepository : public domain::IRestaurantTableRepository
{
  public:
    explicit PostgresRestaurantTableRepository(drogon::orm::DbClientPtr db);
    domain::tables::RestaurantTable create(const domain::tables::RestaurantTable &table) override;
    std::vector<domain::tables::RestaurantTable> listActive() override;
    std::optional<domain::tables::RestaurantTable> findByQrToken(const std::string &qrToken) override;
    std::optional<domain::tables::RestaurantTable> findById(std::int64_t id) override;
    domain::tables::RestaurantTable updateQrToken(std::int64_t id, const std::string &qrToken) override;
    void deactivate(std::int64_t id) override;

  private:
    drogon::orm::DbClientPtr db_;
};

class PostgresOrderRepository : public domain::IOrderRepository
{
  public:
    explicit PostgresOrderRepository(drogon::orm::DbClientPtr db);
    domain::orders::Order create(const domain::orders::Order &order) override;
    std::optional<domain::orders::Order> findById(std::int64_t id) override;
    std::vector<domain::orders::Order> listKitchenActive() override;
    std::vector<domain::orders::Order> listHistory() override;
    std::vector<domain::orders::Order> searchOrders(const std::string &customerName,
                                                    const std::string &tableNumber,
                                                    const std::string &status) override;
    void updateStatus(std::int64_t orderId, domain::OrderStatus status) override;
    void updateOrderItemStatus(std::int64_t itemId, domain::OrderItemStatus status) override;
    bool allItemsReady(std::int64_t orderId) override;

  private:
    std::vector<domain::orders::OrderItem> loadItems(std::int64_t orderId);
    drogon::orm::DbClientPtr db_;
};

class PostgresPaymentRepository : public domain::IPaymentRepository
{
  public:
    explicit PostgresPaymentRepository(drogon::orm::DbClientPtr db);
    std::optional<domain::payments::Payment> findByOrderId(std::int64_t orderId) override;
    domain::payments::Payment upsertPaid(std::int64_t orderId, double amount) override;
    std::vector<domain::payments::Payment> listAll() override;

  private:
    drogon::orm::DbClientPtr db_;
};
}  // namespace starcafe::infrastructure::repositories
