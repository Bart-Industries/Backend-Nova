#pragma once

#include "domain/common/enums.h"

#include <cstdint>
#include <string>
#include <vector>

namespace starcafe::domain::orders
{
struct OrderItemAddonSnapshot
{
    std::int64_t addonId{};
    std::string name;
    double price{};
};

struct OrderItem
{
    std::int64_t id{};
    std::int64_t productId{};
    std::string productName;
    int quantity{};
    double unitPrice{};
    std::string notes;
    OrderItemStatus status{OrderItemStatus::PENDING};
    std::vector<OrderItemAddonSnapshot> addons;
};

struct Order
{
    std::int64_t id{};
    std::int64_t tableId{};
    int tableNumber{};
    std::string customerName;
    OrderStatus status{OrderStatus::PENDING};
    double total{};
    std::vector<OrderItem> items;
    std::string createdAt;
    std::string updatedAt;
};
}  // namespace starcafe::domain::orders
