#include "application/orders/use_cases.h"
#include "domain/common/errors.h"

namespace starcafe::application::orders
{
namespace
{
constexpr std::int64_t kMaxActiveOrdersPerTable = 3;
}

CreateOrderFromTableUseCase::CreateOrderFromTableUseCase(domain::IRestaurantTableRepository &tableRepository,
                                                         domain::IProductRepository &productRepository,
                                                         domain::IAddonRepository &addonRepository,
                                                         domain::IOrderRepository &orderRepository)
    : tableRepository_(tableRepository),
      productRepository_(productRepository),
      addonRepository_(addonRepository),
      orderRepository_(orderRepository)
{
}

domain::orders::Order CreateOrderFromTableUseCase::execute(const CreateOrderFromTableCommand &command)
{
    const auto table = tableRepository_.findByQrToken(command.qrToken);
    if (!table.has_value() || !table->isActive)
    {
        throw domain::DomainError("Invalid table QR token");
    }
    if (command.customerName.empty())
    {
        throw domain::DomainError("Customer name is required");
    }
    if (command.items.empty())
    {
        throw domain::DomainError("Order must include at least one item");
    }
    if (orderRepository_.countActiveByTableId(table->id) >= kMaxActiveOrdersPerTable)
    {
        throw domain::DomainError("This table already has the maximum number of active orders");
    }

    domain::orders::Order order;
    order.businessId = table->businessId;
    order.tableId = table->id;
    order.tableNumber = table->tableNumber;
    order.customerName = command.customerName;
    order.status = domain::OrderStatus::PENDING;

    double total = 0.0;
    for (const auto &inputItem : command.items)
    {
        const auto product = productRepository_.findById(inputItem.productId);
        if (!product.has_value() || product->businessId != table->businessId || !product->isActive || !product->isAvailable)
        {
            throw domain::DomainError("Product is not available");
        }

        domain::orders::OrderItem item;
        item.productId = product->id;
        item.productName = product->name;
        item.quantity = inputItem.quantity;
        item.unitPrice = product->price;
        item.notes = inputItem.notes;
        item.status = domain::OrderItemStatus::PENDING;

        double addonsTotal = 0.0;
        for (const auto addonId : inputItem.addonIds)
        {
            const auto addon = addonRepository_.findById(addonId);
            if (!addon.has_value() || addon->businessId != table->businessId || !addon->isActive)
            {
                throw domain::DomainError("Addon is not available");
            }
            item.addons.push_back({addon->id, addon->name, addon->price});
            addonsTotal += addon->price;
        }

        total += (item.unitPrice + addonsTotal) * item.quantity;
        order.items.push_back(item);
    }

    order.total = total;
    return orderRepository_.create(order);
}

GetOrderStatusForCustomerUseCase::GetOrderStatusForCustomerUseCase(domain::IOrderRepository &orderRepository)
    : orderRepository_(orderRepository)
{
}

domain::orders::Order GetOrderStatusForCustomerUseCase::execute(std::int64_t orderId)
{
    const auto order = orderRepository_.findById(orderId);
    if (!order.has_value())
    {
        throw domain::DomainError("Order not found");
    }
    return *order;
}

GetPublicTableSessionUseCase::GetPublicTableSessionUseCase(domain::IRestaurantTableRepository &tableRepository,
                                                           domain::IOrderRepository &orderRepository,
                                                           domain::IBusinessRepository &businessRepository)
    : tableRepository_(tableRepository), orderRepository_(orderRepository), businessRepository_(businessRepository)
{
}

PublicTableSession GetPublicTableSessionUseCase::execute(const std::string &qrToken)
{
    const auto table = tableRepository_.findByQrToken(qrToken);
    if (!table.has_value() || !table->isActive)
    {
        throw domain::DomainError("Invalid table QR token");
    }

    PublicTableSession session;
    const auto business = businessRepository_.findById(table->businessId);
    if (!business.has_value() || !business->isActive)
    {
        throw domain::DomainError("Business not found");
    }
    session.business = *business;
    session.table = *table;
    session.activeOrders = orderRepository_.listActiveByTableId(table->id);
    session.activeOrdersCount = static_cast<std::int64_t>(session.activeOrders.size());
    session.remainingSlots = kMaxActiveOrdersPerTable - session.activeOrdersCount;
    if (session.remainingSlots < 0)
    {
        session.remainingSlots = 0;
    }
    session.canCreateMoreOrders = session.activeOrdersCount < kMaxActiveOrdersPerTable;
    return session;
}

GetKitchenOrdersUseCase::GetKitchenOrdersUseCase(domain::IOrderRepository &orderRepository) : orderRepository_(orderRepository) {}
std::vector<domain::orders::Order> GetKitchenOrdersUseCase::execute(std::int64_t businessId) { return orderRepository_.listKitchenActive(businessId); }

StartPreparingOrderUseCase::StartPreparingOrderUseCase(domain::IOrderRepository &orderRepository) : orderRepository_(orderRepository) {}
void StartPreparingOrderUseCase::execute(std::int64_t businessId, std::int64_t orderId)
{
    const auto order = orderRepository_.findById(orderId);
    if (!order.has_value() || order->businessId != businessId)
    {
        throw domain::DomainError("Order not found");
    }
    if (order->status != domain::OrderStatus::PENDING)
    {
        throw domain::DomainError("Only pending orders can move to preparing");
    }
    orderRepository_.updateStatus(orderId, domain::OrderStatus::PREPARING);
}

MarkOrderItemReadyUseCase::MarkOrderItemReadyUseCase(domain::IOrderRepository &orderRepository) : orderRepository_(orderRepository) {}
void MarkOrderItemReadyUseCase::execute(std::int64_t businessId, std::int64_t itemId)
{
    (void)businessId;
    orderRepository_.updateOrderItemStatus(itemId, domain::OrderItemStatus::READY);
}

MarkOrderReadyUseCase::MarkOrderReadyUseCase(domain::IOrderRepository &orderRepository) : orderRepository_(orderRepository) {}
void MarkOrderReadyUseCase::execute(std::int64_t businessId, std::int64_t orderId)
{
    const auto order = orderRepository_.findById(orderId);
    if (!order.has_value() || order->businessId != businessId)
    {
        throw domain::DomainError("Order not found");
    }
    if (!orderRepository_.allItemsReady(orderId))
    {
        throw domain::DomainError("All order items must be READY before closing the order");
    }
    orderRepository_.updateStatus(orderId, domain::OrderStatus::READY);
}

GetOrdersHistoryUseCase::GetOrdersHistoryUseCase(domain::IOrderRepository &orderRepository) : orderRepository_(orderRepository) {}
std::vector<domain::orders::Order> GetOrdersHistoryUseCase::execute(std::int64_t businessId) { return orderRepository_.listHistory(businessId); }

CancelOrderUseCase::CancelOrderUseCase(domain::IOrderRepository &orderRepository) : orderRepository_(orderRepository) {}
void CancelOrderUseCase::execute(std::int64_t businessId, std::int64_t orderId)
{
    const auto order = orderRepository_.findById(orderId);
    if (!order.has_value() || order->businessId != businessId)
    {
        throw domain::DomainError("Order not found");
    }
    orderRepository_.updateStatus(orderId, domain::OrderStatus::CANCELLED);
}
}  // namespace starcafe::application::orders
