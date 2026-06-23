#pragma once

#include "application/identity/use_cases.h"
#include "application/menu/use_cases.h"
#include "application/orders/use_cases.h"
#include "application/payments/use_cases.h"
#include "application/tables/use_cases.h"

namespace starcafe::interfaces::rest
{
struct ServiceRegistry
{
    domain::IUserRepository *userRepository{};
    domain::IProductRepository *productRepository{};
    domain::IAddonRepository *addonRepository{};
    application::identity::RegisterUserUseCase *registerUser{};
    application::identity::LoginUseCase *login{};
    application::identity::GetCurrentUserUseCase *getCurrentUser{};
    application::menu::CreateCategoryUseCase *createCategory{};
    application::menu::ListCategoriesUseCase *listCategories{};
    application::menu::CreateProductUseCase *createProduct{};
    application::menu::UpdateProductUseCase *updateProduct{};
    application::menu::MarkProductUnavailableUseCase *markProductUnavailable{};
    application::menu::DeactivateProductUseCase *deactivateProduct{};
    application::menu::CreateAddonUseCase *createAddon{};
    application::menu::AssignAddonToProductUseCase *assignAddonToProduct{};
    application::menu::GetPublicMenuUseCase *getPublicMenu{};
    application::tables::CreateTableUseCase *createTable{};
    application::tables::GenerateQrTokenUseCase *generateQrToken{};
    application::tables::GetTableByQrTokenUseCase *getTableByQrToken{};
    application::tables::ListTablesUseCase *listTables{};
    application::tables::DeactivateTableUseCase *deactivateTable{};
    application::orders::CreateOrderFromTableUseCase *createOrderFromTable{};
    application::orders::GetOrderStatusForCustomerUseCase *getOrderStatus{};
    application::orders::GetKitchenOrdersUseCase *getKitchenOrders{};
    application::orders::StartPreparingOrderUseCase *startPreparingOrder{};
    application::orders::MarkOrderItemReadyUseCase *markOrderItemReady{};
    application::orders::MarkOrderReadyUseCase *markOrderReady{};
    application::orders::GetOrdersHistoryUseCase *getOrdersHistory{};
    application::orders::CancelOrderUseCase *cancelOrder{};
    application::payments::SearchOrdersForCashierUseCase *searchOrdersForCashier{};
    application::payments::PayOrderUseCase *payOrder{};
    application::payments::ListPaymentsUseCase *listPayments{};
};

ServiceRegistry &services();
}  // namespace starcafe::interfaces::rest
