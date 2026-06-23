#include "application/identity/use_cases.h"
#include "application/menu/use_cases.h"
#include "application/orders/use_cases.h"
#include "application/payments/use_cases.h"
#include "application/tables/use_cases.h"
#include "infrastructure/config/app_config.h"
#include "infrastructure/database/database.h"
#include "infrastructure/qr/qr_token_service.h"
#include "infrastructure/repositories/postgres_repositories.h"
#include "infrastructure/security/auth_filters.h"
#include "infrastructure/security/jwt_service.h"
#include "infrastructure/security/password_hasher.h"
#include "interfaces/rest/service_registry.h"

#include <drogon/drogon.h>

int main()
{
    using namespace starcafe;

    const auto config = infrastructure::config::AppConfig::load();
    auto db = infrastructure::database::createDbClient(config.databaseUrl);

    infrastructure::security::PasswordHasher passwordHasher(config.bcryptCost);
    infrastructure::security::JwtService jwtService(config.jwtSecret, config.jwtExpiresIn);
    infrastructure::qr::QrTokenService qrTokenService;

    infrastructure::repositories::PostgresUserRepository userRepository(db);
    infrastructure::repositories::PostgresCategoryRepository categoryRepository(db);
    infrastructure::repositories::PostgresAddonRepository addonRepository(db);
    infrastructure::repositories::PostgresProductRepository productRepository(db, addonRepository);
    infrastructure::repositories::PostgresRestaurantTableRepository tableRepository(db);
    infrastructure::repositories::PostgresOrderRepository orderRepository(db);
    infrastructure::repositories::PostgresPaymentRepository paymentRepository(db);

    application::identity::RegisterUserUseCase registerUser(userRepository, passwordHasher);
    application::identity::LoginUseCase login(userRepository, passwordHasher, jwtService);
    application::identity::GetCurrentUserUseCase getCurrentUser(userRepository);
    application::menu::CreateCategoryUseCase createCategory(categoryRepository);
    application::menu::ListCategoriesUseCase listCategories(categoryRepository);
    application::menu::CreateProductUseCase createProduct(productRepository);
    application::menu::UpdateProductUseCase updateProduct(productRepository);
    application::menu::MarkProductUnavailableUseCase markProductUnavailable(productRepository);
    application::menu::DeactivateProductUseCase deactivateProduct(productRepository);
    application::menu::CreateAddonUseCase createAddon(addonRepository);
    application::menu::AssignAddonToProductUseCase assignAddonToProduct(productRepository);
    application::menu::GetPublicMenuUseCase getPublicMenu(productRepository);
    application::tables::CreateTableUseCase createTable(tableRepository, qrTokenService);
    application::tables::GenerateQrTokenUseCase generateQrToken(tableRepository, qrTokenService);
    application::tables::GetTableByQrTokenUseCase getTableByQrToken(tableRepository);
    application::tables::ListTablesUseCase listTables(tableRepository);
    application::tables::DeactivateTableUseCase deactivateTable(tableRepository);
    application::orders::CreateOrderFromTableUseCase createOrderFromTable(tableRepository, productRepository, addonRepository, orderRepository);
    application::orders::GetOrderStatusForCustomerUseCase getOrderStatus(orderRepository);
    application::orders::GetKitchenOrdersUseCase getKitchenOrders(orderRepository);
    application::orders::StartPreparingOrderUseCase startPreparingOrder(orderRepository);
    application::orders::MarkOrderItemReadyUseCase markOrderItemReady(orderRepository);
    application::orders::MarkOrderReadyUseCase markOrderReady(orderRepository);
    application::orders::GetOrdersHistoryUseCase getOrdersHistory(orderRepository);
    application::orders::CancelOrderUseCase cancelOrder(orderRepository);
    application::payments::SearchOrdersForCashierUseCase searchOrdersForCashier(orderRepository);
    application::payments::PayOrderUseCase payOrder(orderRepository, paymentRepository);
    application::payments::ListPaymentsUseCase listPayments(paymentRepository);

    auto &registry = interfaces::rest::services();
    registry.userRepository = &userRepository;
    registry.productRepository = &productRepository;
    registry.addonRepository = &addonRepository;
    registry.registerUser = &registerUser;
    registry.login = &login;
    registry.getCurrentUser = &getCurrentUser;
    registry.createCategory = &createCategory;
    registry.listCategories = &listCategories;
    registry.createProduct = &createProduct;
    registry.updateProduct = &updateProduct;
    registry.markProductUnavailable = &markProductUnavailable;
    registry.deactivateProduct = &deactivateProduct;
    registry.createAddon = &createAddon;
    registry.assignAddonToProduct = &assignAddonToProduct;
    registry.getPublicMenu = &getPublicMenu;
    registry.createTable = &createTable;
    registry.generateQrToken = &generateQrToken;
    registry.getTableByQrToken = &getTableByQrToken;
    registry.listTables = &listTables;
    registry.deactivateTable = &deactivateTable;
    registry.createOrderFromTable = &createOrderFromTable;
    registry.getOrderStatus = &getOrderStatus;
    registry.getKitchenOrders = &getKitchenOrders;
    registry.startPreparingOrder = &startPreparingOrder;
    registry.markOrderItemReady = &markOrderItemReady;
    registry.markOrderReady = &markOrderReady;
    registry.getOrdersHistory = &getOrdersHistory;
    registry.cancelOrder = &cancelOrder;
    registry.searchOrdersForCashier = &searchOrdersForCashier;
    registry.payOrder = &payOrder;
    registry.listPayments = &listPayments;

    infrastructure::security::configureJwt(&jwtService);

    drogon::app().addListener("0.0.0.0", config.appPort);
    drogon::app().setThreadNum(1);
    drogon::app().run();
    return 0;
}
