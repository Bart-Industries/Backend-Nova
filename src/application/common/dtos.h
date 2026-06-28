#pragma once

#include "domain/common/enums.h"
#include "domain/menu/menu_models.h"
#include "domain/orders/order.h"
#include "domain/tables/restaurant_table.h"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace starcafe::application
{
struct RegisterUserCommand
{
    std::string name;
    std::string email;
    std::string password;
    std::optional<std::int64_t> businessId;
    domain::UserRole role{domain::UserRole::KITCHEN};
};

struct LoginCommand
{
    std::string email;
    std::string password;
};

struct CreateBusinessCommand
{
    std::string name;
    std::string slug;
    std::string logoUrl;
    std::string primaryColor;
};

struct CreateCategoryCommand
{
    std::string name;
    std::string description;
};

struct CreateProductCommand
{
    std::int64_t businessId{};
    std::int64_t categoryId{};
    std::string name;
    std::string description;
    double price{};
};

struct UpdateProductCommand
{
    std::int64_t businessId{};
    std::int64_t categoryId{};
    std::string name;
    std::string description;
    double price{};
    bool isAvailable{true};
};

struct CreateAddonCommand
{
    std::int64_t businessId{};
    std::string name;
    double price{};
};

struct UploadProductImageCommand
{
    std::int64_t productId{};
    std::string originalFileName;
    std::string mimeType;
    std::int64_t fileSize{};
    std::string tempFilePath;
    bool deleteOldPhysicalFile{false};
};

struct CreateTableCommand
{
    std::int64_t businessId{};
    int tableNumber{};
};

struct CreateOrderItemCommand
{
    std::int64_t productId{};
    int quantity{};
    std::string notes;
    std::vector<std::int64_t> addonIds;
};

struct CreateOrderFromTableCommand
{
    std::string qrToken;
    std::string customerName;
    std::vector<CreateOrderItemCommand> items;
};

struct PayOrderCommand
{
    std::int64_t businessId{};
    std::int64_t orderId{};
    double amount{};
};

struct AuthPayload
{
    std::string token;
    std::int64_t userId{};
    std::optional<std::int64_t> businessId;
    std::string role;
    std::string name;
    std::string email;
};
}  // namespace starcafe::application
