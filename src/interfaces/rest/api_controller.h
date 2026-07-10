#pragma once

#include <drogon/HttpController.h>

namespace starcafe::interfaces::rest
{
class ApiController : public drogon::HttpController<ApiController>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(ApiController::registerUser, "/api/v1/auth/register", drogon::Post);
    ADD_METHOD_TO(ApiController::login, "/api/v1/auth/login", drogon::Post);
    ADD_METHOD_TO(ApiController::me, "/api/v1/auth/me", drogon::Get, "starcafe::infrastructure::security::JwtAuthFilter");
    ADD_METHOD_TO(ApiController::listBusinesses, "/api/v1/super-admin/businesses", drogon::Get, "starcafe::infrastructure::security::JwtAuthFilter");
    ADD_METHOD_TO(ApiController::createBusiness, "/api/v1/super-admin/businesses", drogon::Post, "starcafe::infrastructure::security::JwtAuthFilter");
    ADD_METHOD_TO(ApiController::getTableByQr, "/api/v1/tables/qr/{1}", drogon::Get);
    ADD_METHOD_TO(ApiController::getPublicTableSession, "/api/v1/public/tables/{1}/session", drogon::Get);
    ADD_METHOD_TO(ApiController::getPublicMenu, "/api/v1/public/menu", drogon::Get);
    ADD_METHOD_TO(ApiController::createOrderFromTable, "/api/v1/public/tables/{1}/orders", drogon::Post);
    ADD_METHOD_TO(ApiController::getOrderStatus, "/api/v1/public/orders/{1}/status", drogon::Get);
    ADD_METHOD_TO(ApiController::getKitchenOrders, "/api/v1/kitchen/orders", drogon::Get, "starcafe::infrastructure::security::KitchenFilter");
    ADD_METHOD_TO(ApiController::startPreparingOrder, "/api/v1/kitchen/orders/{1}/preparing", drogon::Patch, "starcafe::infrastructure::security::KitchenFilter");
    ADD_METHOD_TO(ApiController::markOrderItemReady, "/api/v1/kitchen/order-items/{1}/ready", drogon::Patch, "starcafe::infrastructure::security::KitchenFilter");
    ADD_METHOD_TO(ApiController::markOrderReady, "/api/v1/kitchen/orders/{1}/ready", drogon::Patch, "starcafe::infrastructure::security::KitchenFilter");
    ADD_METHOD_TO(ApiController::getOrdersHistory, "/api/v1/kitchen/orders/history", drogon::Get, "starcafe::infrastructure::security::KitchenFilter");
    ADD_METHOD_TO(ApiController::listUsers, "/api/v1/admin/users", drogon::Get, "starcafe::infrastructure::security::AdminFilter");
    ADD_METHOD_TO(ApiController::createAdminUser, "/api/v1/admin/users", drogon::Post, "starcafe::infrastructure::security::AdminFilter");
    ADD_METHOD_TO(ApiController::deactivateUser, "/api/v1/admin/users/{1}/deactivate", drogon::Patch, "starcafe::infrastructure::security::AdminFilter");
    ADD_METHOD_TO(ApiController::listTables, "/api/v1/admin/tables", drogon::Get, "starcafe::infrastructure::security::AdminFilter");
    ADD_METHOD_TO(ApiController::createTable, "/api/v1/admin/tables", drogon::Post, "starcafe::infrastructure::security::AdminFilter");
    ADD_METHOD_TO(ApiController::regenerateTableQr, "/api/v1/admin/tables/{1}/regenerate-qr", drogon::Patch, "starcafe::infrastructure::security::AdminFilter");
    ADD_METHOD_TO(ApiController::deactivateTable, "/api/v1/admin/tables/{1}/deactivate", drogon::Patch, "starcafe::infrastructure::security::AdminFilter");
    ADD_METHOD_TO(ApiController::listCategories, "/api/v1/admin/categories", drogon::Get, "starcafe::infrastructure::security::AdminFilter");
    ADD_METHOD_TO(ApiController::createCategory, "/api/v1/admin/categories", drogon::Post, "starcafe::infrastructure::security::AdminFilter");
    ADD_METHOD_TO(ApiController::listProducts, "/api/v1/admin/products", drogon::Get, "starcafe::infrastructure::security::AdminFilter");
    ADD_METHOD_TO(ApiController::createProduct, "/api/v1/admin/products", drogon::Post, "starcafe::infrastructure::security::AdminFilter");
    ADD_METHOD_TO(ApiController::updateProduct, "/api/v1/admin/products/{1}", drogon::Patch, "starcafe::infrastructure::security::AdminFilter");
    ADD_METHOD_TO(ApiController::activateProduct, "/api/v1/admin/products/{1}/activate", drogon::Patch, "starcafe::infrastructure::security::AdminFilter");
    ADD_METHOD_TO(ApiController::markProductUnavailable, "/api/v1/admin/products/{1}/unavailable", drogon::Patch, "starcafe::infrastructure::security::AdminFilter");
    ADD_METHOD_TO(ApiController::deactivateProduct, "/api/v1/admin/products/{1}/deactivate", drogon::Patch, "starcafe::infrastructure::security::AdminFilter");
    ADD_METHOD_TO(ApiController::listAddons, "/api/v1/admin/addons", drogon::Get, "starcafe::infrastructure::security::AdminFilter");
    ADD_METHOD_TO(ApiController::createAddon, "/api/v1/admin/addons", drogon::Post, "starcafe::infrastructure::security::AdminFilter");
    ADD_METHOD_TO(ApiController::assignAddonToProduct, "/api/v1/admin/products/{1}/addons/{2}", drogon::Post, "starcafe::infrastructure::security::AdminFilter");
    ADD_METHOD_TO(ApiController::listAdminOrders, "/api/v1/admin/orders", drogon::Get, "starcafe::infrastructure::security::AdminFilter");
    ADD_METHOD_TO(ApiController::listAdminOrdersHistory, "/api/v1/admin/orders/history", drogon::Get, "starcafe::infrastructure::security::AdminFilter");
    ADD_METHOD_TO(ApiController::cancelOrder, "/api/v1/admin/orders/{1}/cancel", drogon::Patch, "starcafe::infrastructure::security::AdminFilter");
    ADD_METHOD_TO(ApiController::searchCashierOrders, "/api/v1/admin/cashier/orders/search", drogon::Get, "starcafe::infrastructure::security::AdminFilter");
    ADD_METHOD_TO(ApiController::payOrder, "/api/v1/admin/cashier/orders/{1}/pay", drogon::Post, "starcafe::infrastructure::security::AdminFilter");
    METHOD_LIST_END

    void registerUser(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void login(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void me(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void listBusinesses(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void createBusiness(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void getTableByQr(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string qrToken);
    void getPublicTableSession(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string qrToken);
    void getPublicMenu(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void createOrderFromTable(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string qrToken);
    void getOrderStatus(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string orderId);
    void getKitchenOrders(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void startPreparingOrder(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string orderId);
    void markOrderItemReady(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string itemId);
    void markOrderReady(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string orderId);
    void getOrdersHistory(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void listUsers(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void createAdminUser(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void deactivateUser(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string id);
    void listTables(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void createTable(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void regenerateTableQr(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string id);
    void deactivateTable(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string id);
    void listCategories(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void createCategory(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void listProducts(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void createProduct(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void updateProduct(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string id);
    void activateProduct(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string id);
    void markProductUnavailable(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string id);
    void deactivateProduct(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string id);
    void listAddons(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void createAddon(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void assignAddonToProduct(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string productId, std::string addonId);
    void listAdminOrders(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void listAdminOrdersHistory(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void cancelOrder(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string orderId);
    void searchCashierOrders(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void payOrder(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string orderId);
};
}  // namespace starcafe::interfaces::rest
