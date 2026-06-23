#include "application/orders/use_cases.h"
#include "domain/common/errors.h"

namespace starcafe::application::orders
{
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

    domain::orders::Order order;
    order.tableId = table->id;
    order.tableNumber = table->tableNumber;
    order.customerName = command.customerName;
    order.status = domain::OrderStatus::PENDING;

    double total = 0.0;
    for (const auto &inputItem : command.items)
    {
        const auto product = productRepository_.findById(inputItem.productId);
        if (!product.has_value() || !product->isActive || !product->isAvailable)
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
            if (!addon.has_value() || !addon->isActive)
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

GetKitchenOrdersUseCase::GetKitchenOrdersUseCase(domain::IOrderRepository &orderRepository) : orderRepository_(orderRepository) {}
std::vector<domain::orders::Order> GetKitchenOrdersUseCase::execute() { return orderRepository_.listKitchenActive(); }

StartPreparingOrderUseCase::StartPreparingOrderUseCase(domain::IOrderRepository &orderRepository) : orderRepository_(orderRepository) {}
void StartPreparingOrderUseCase::execute(std::int64_t orderId)
{
    const auto order = orderRepository_.findById(orderId);
    if (!order.has_value() || order->status != domain::OrderStatus::PENDING)
    {
        throw domain::DomainError("Only pending orders can move to preparing");
    }
    orderRepository_.updateStatus(orderId, domain::OrderStatus::PREPARING);
}

MarkOrderItemReadyUseCase::MarkOrderItemReadyUseCase(domain::IOrderRepository &orderRepository) : orderRepository_(orderRepository) {}
void MarkOrderItemReadyUseCase::execute(std::int64_t itemId) { orderRepository_.updateOrderItemStatus(itemId, domain::OrderItemStatus::READY); }

MarkOrderReadyUseCase::MarkOrderReadyUseCase(domain::IOrderRepository &orderRepository) : orderRepository_(orderRepository) {}
void MarkOrderReadyUseCase::execute(std::int64_t orderId)
{
    if (!orderRepository_.allItemsReady(orderId))
    {
        throw domain::DomainError("All order items must be READY before closing the order");
    }
    orderRepository_.updateStatus(orderId, domain::OrderStatus::READY);
}

GetOrdersHistoryUseCase::GetOrdersHistoryUseCase(domain::IOrderRepository &orderRepository) : orderRepository_(orderRepository) {}
std::vector<domain::orders::Order> GetOrdersHistoryUseCase::execute() { return orderRepository_.listHistory(); }

CancelOrderUseCase::CancelOrderUseCase(domain::IOrderRepository &orderRepository) : orderRepository_(orderRepository) {}
void CancelOrderUseCase::execute(std::int64_t orderId)
{
    const auto order = orderRepository_.findById(orderId);
    if (!order.has_value())
    {
        throw domain::DomainError("Order not found");
    }
    orderRepository_.updateStatus(orderId, domain::OrderStatus::CANCELLED);
}
}  // namespace starcafe::application::orders
