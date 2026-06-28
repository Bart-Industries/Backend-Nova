#pragma once

#include <stdexcept>
#include <string>

namespace starcafe::domain
{
enum class UserRole
{
    SUPER_ADMIN,
    ADMIN,
    KITCHEN
};

enum class OrderStatus
{
    PENDING,
    PREPARING,
    READY,
    PAID,
    CANCELLED
};

enum class OrderItemStatus
{
    PENDING,
    READY
};

enum class PaymentStatus
{
    PENDING,
    PAID,
    CANCELLED
};

inline std::string toString(UserRole role)
{
    switch (role)
    {
    case UserRole::SUPER_ADMIN:
        return "SUPER_ADMIN";
    case UserRole::ADMIN:
        return "ADMIN";
    case UserRole::KITCHEN:
        return "KITCHEN";
    }
    throw std::invalid_argument("Unknown user role");
}

inline std::string toString(OrderStatus status)
{
    switch (status)
    {
    case OrderStatus::PENDING:
        return "PENDING";
    case OrderStatus::PREPARING:
        return "PREPARING";
    case OrderStatus::READY:
        return "READY";
    case OrderStatus::PAID:
        return "PAID";
    case OrderStatus::CANCELLED:
        return "CANCELLED";
    }
    throw std::invalid_argument("Unknown order status");
}

inline std::string toString(OrderItemStatus status)
{
    switch (status)
    {
    case OrderItemStatus::PENDING:
        return "PENDING";
    case OrderItemStatus::READY:
        return "READY";
    }
    throw std::invalid_argument("Unknown order item status");
}

inline std::string toString(PaymentStatus status)
{
    switch (status)
    {
    case PaymentStatus::PENDING:
        return "PENDING";
    case PaymentStatus::PAID:
        return "PAID";
    case PaymentStatus::CANCELLED:
        return "CANCELLED";
    }
    throw std::invalid_argument("Unknown payment status");
}

inline UserRole userRoleFromString(const std::string &value)
{
    if (value == "SUPER_ADMIN")
        return UserRole::SUPER_ADMIN;
    if (value == "ADMIN")
        return UserRole::ADMIN;
    if (value == "KITCHEN")
        return UserRole::KITCHEN;
    throw std::invalid_argument("Invalid user role");
}

inline OrderStatus orderStatusFromString(const std::string &value)
{
    if (value == "PENDING")
        return OrderStatus::PENDING;
    if (value == "PREPARING")
        return OrderStatus::PREPARING;
    if (value == "READY")
        return OrderStatus::READY;
    if (value == "PAID")
        return OrderStatus::PAID;
    if (value == "CANCELLED")
        return OrderStatus::CANCELLED;
    throw std::invalid_argument("Invalid order status");
}

inline OrderItemStatus orderItemStatusFromString(const std::string &value)
{
    if (value == "PENDING")
        return OrderItemStatus::PENDING;
    if (value == "READY")
        return OrderItemStatus::READY;
    throw std::invalid_argument("Invalid order item status");
}

inline PaymentStatus paymentStatusFromString(const std::string &value)
{
    if (value == "PENDING")
        return PaymentStatus::PENDING;
    if (value == "PAID")
        return PaymentStatus::PAID;
    if (value == "CANCELLED")
        return PaymentStatus::CANCELLED;
    throw std::invalid_argument("Invalid payment status");
}
}  // namespace starcafe::domain
