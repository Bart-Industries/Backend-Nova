#include "infrastructure/repositories/postgres_repositories.h"

#include "domain/common/enums.h"
#include "domain/common/errors.h"

#include <algorithm>

using namespace drogon::orm;

namespace starcafe::infrastructure::repositories
{
namespace
{
std::string nullableString(const Row &row, const char *columnName)
{
    const auto field = row[columnName];
    return field.isNull() ? "" : field.as<std::string>();
}

std::string cleanText(std::string value)
{
    value.erase(std::remove(value.begin(), value.end(), '\0'), value.end());
    return value;
}

domain::identity::User mapUser(const Row &row)
{
    return {row["id"].as<std::int64_t>(),
            row["name"].as<std::string>(),
            row["email"].as<std::string>(),
            row["password_hash"].as<std::string>(),
            domain::userRoleFromString(row["role"].as<std::string>()),
            row["is_active"].as<bool>(),
            {}};
}

domain::menu::Category mapCategory(const Row &row)
{
    return {row["id"].as<std::int64_t>(), row["name"].as<std::string>(), nullableString(row, "description"), row["is_active"].as<bool>()};
}

domain::menu::Addon mapAddon(const Row &row)
{
    return {row["id"].as<std::int64_t>(), row["name"].as<std::string>(), row["price"].as<double>(), row["is_active"].as<bool>()};
}

domain::menu::Product mapProduct(const Row &row)
{
    return {row["id"].as<std::int64_t>(),
            row["category_id"].as<std::int64_t>(),
            row["name"].as<std::string>(),
            nullableString(row, "description"),
            row["price"].as<double>(),
            row["is_available"].as<bool>(),
            row["is_active"].as<bool>(),
            {},
            std::nullopt};
}

domain::menu::ProductImage mapProductImage(const Row &row)
{
    return {row["id"].as<std::int64_t>(),
            row["product_id"].as<std::int64_t>(),
            row["file_name"].as<std::string>(),
            row["file_path"].as<std::string>(),
            row["mime_type"].as<std::string>(),
            row["file_size"].as<std::int64_t>(),
            row["is_main"].as<bool>(),
            row["created_at"].as<std::string>()};
}

domain::tables::RestaurantTable mapTable(const Row &row)
{
    return {row["id"].as<std::int64_t>(), row["table_number"].as<int>(), row["qr_token"].as<std::string>(), row["is_active"].as<bool>()};
}

domain::orders::Order mapOrderBase(const Row &row)
{
    domain::orders::Order order;
    order.id = row["id"].as<std::int64_t>();
    order.tableId = row["table_id"].as<std::int64_t>();
    try
    {
        order.tableNumber = row["table_number"].as<int>();
    }
    catch (const std::exception &)
    {
        order.tableNumber = 0;
    }
    order.customerName = row["customer_name"].as<std::string>();
    order.status = domain::orderStatusFromString(row["status"].as<std::string>());
    order.total = row["total"].as<double>();
    order.createdAt = row["created_at"].as<std::string>();
    return order;
}
}  // namespace

PostgresUserRepository::PostgresUserRepository(DbClientPtr db) : db_(std::move(db)) {}

const std::string &PostgresUserRepository::passwordColumn()
{
    if (!passwordColumn_.empty())
    {
        return passwordColumn_;
    }

    const auto result = db_->execSqlSync(
        "select column_name from information_schema.columns "
        "where table_schema = 'public' and table_name = 'users' and column_name in ('password_hash', 'password')");

    for (const auto &row : result)
    {
        const auto column = row["column_name"].as<std::string>();
        if (column == "password_hash" || column == "password")
        {
            passwordColumn_ = column;
            return passwordColumn_;
        }
    }

    throw domain::DomainError("Users table does not contain password_hash or password column");
}

std::optional<domain::identity::User> PostgresUserRepository::findByEmail(const std::string &email)
{
    const auto sql = "select id, name, email, " + passwordColumn() + " as password_hash, role, is_active from users where email = $1 limit 1";
    const auto result = db_->execSqlSync(sql, email);
    if (result.empty())
        return std::nullopt;
    return mapUser(result[0]);
}
std::optional<domain::identity::User> PostgresUserRepository::findById(std::int64_t id)
{
    const auto sql = "select id, name, email, " + passwordColumn() + " as password_hash, role, is_active from users where id = $1 limit 1";
    const auto result = db_->execSqlSync(sql, id);
    if (result.empty())
        return std::nullopt;
    return mapUser(result[0]);
}
std::vector<domain::identity::User> PostgresUserRepository::listActive()
{
    std::vector<domain::identity::User> users;
    const auto sql = "select id, name, email, " + passwordColumn() + " as password_hash, role, is_active from users where is_active = true order by id asc";
    for (const auto &row : db_->execSqlSync(sql))
        users.push_back(mapUser(row));
    return users;
}
std::int64_t PostgresUserRepository::countActiveAdmins()
{
    const auto result = db_->execSqlSync("select count(*) as count from users where role = 'ADMIN' and is_active = true");
    return result[0]["count"].as<std::int64_t>();
}
domain::identity::User PostgresUserRepository::create(const domain::identity::User &user)
{
    const auto sql = "insert into users (name, email, " + passwordColumn() + ", role, is_active) "
                     "values ($1, $2, $3, $4, $5) "
                     "returning id, name, email, " +
                     passwordColumn() + " as password_hash, role, is_active";
    const auto result = db_->execSqlSync(sql, user.name, user.email, user.passwordHash, domain::toString(user.role), user.isActive);
    return mapUser(result[0]);
}
void PostgresUserRepository::deactivate(std::int64_t id) { db_->execSqlSync("update users set is_active = false where id = $1", id); }

PostgresCategoryRepository::PostgresCategoryRepository(DbClientPtr db) : db_(std::move(db)) {}

bool PostgresCategoryRepository::hasDescriptionColumn()
{
    if (categorySchemaResolved_)
    {
        return hasDescriptionColumn_;
    }

    const auto result = db_->execSqlSync(
        "select 1 from information_schema.columns "
        "where table_schema = 'public' and table_name = 'categories' and column_name = 'description' limit 1");

    hasDescriptionColumn_ = !result.empty();
    categorySchemaResolved_ = true;
    return hasDescriptionColumn_;
}

std::vector<domain::menu::Category> PostgresCategoryRepository::listAll(bool onlyActive)
{
    std::vector<domain::menu::Category> categories;
    const auto sql = hasDescriptionColumn()
                         ? (onlyActive ? "select id, name, description, is_active from categories where is_active = true order by id asc"
                                       : "select id, name, description, is_active from categories order by id asc")
                         : (onlyActive ? "select id, name, '' as description, is_active from categories where is_active = true order by id asc"
                                       : "select id, name, '' as description, is_active from categories order by id asc");
    for (const auto &row : db_->execSqlSync(sql))
        categories.push_back(mapCategory(row));
    return categories;
}
domain::menu::Category PostgresCategoryRepository::create(const domain::menu::Category &category)
{
    const auto result = hasDescriptionColumn()
                            ? db_->execSqlSync("insert into categories (name, description, is_active) values ($1, $2, $3) returning id, name, description, is_active",
                                               category.name,
                                               category.description,
                                               category.isActive)
                            : db_->execSqlSync("insert into categories (name, is_active) values ($1, $2) returning id, name, '' as description, is_active",
                                               category.name,
                                               category.isActive);
    return mapCategory(result[0]);
}
domain::menu::Category PostgresCategoryRepository::update(std::int64_t id, const domain::menu::Category &category)
{
    const auto result = hasDescriptionColumn()
                            ? db_->execSqlSync("update categories set name = $1, description = $2 where id = $3 returning id, name, description, is_active",
                                               category.name,
                                               category.description,
                                               id)
                            : db_->execSqlSync("update categories set name = $1 where id = $2 returning id, name, '' as description, is_active",
                                               category.name,
                                               id);
    return mapCategory(result[0]);
}

PostgresAddonRepository::PostgresAddonRepository(DbClientPtr db) : db_(std::move(db)) {}
std::optional<domain::menu::Addon> PostgresAddonRepository::findById(std::int64_t id)
{
    const auto result = db_->execSqlSync("select id, name, price, is_active from addons where id = $1 limit 1", id);
    if (result.empty())
        return std::nullopt;
    return mapAddon(result[0]);
}
std::vector<domain::menu::Addon> PostgresAddonRepository::listAll(bool onlyActive)
{
    std::vector<domain::menu::Addon> addons;
    const auto sql = onlyActive ? "select id, name, price, is_active from addons where is_active = true order by id asc"
                                : "select id, name, price, is_active from addons order by id asc";
    for (const auto &row : db_->execSqlSync(sql))
        addons.push_back(mapAddon(row));
    return addons;
}
domain::menu::Addon PostgresAddonRepository::create(const domain::menu::Addon &addon)
{
    const auto result = db_->execSqlSync("insert into addons (name, price, is_active) values ($1, $2, $3) returning id, name, price, is_active",
                                         addon.name,
                                         addon.price,
                                         addon.isActive);
    return mapAddon(result[0]);
}
domain::menu::Addon PostgresAddonRepository::update(std::int64_t id, const domain::menu::Addon &addon)
{
    const auto result = db_->execSqlSync("update addons set name = $1, price = $2 where id = $3 returning id, name, price, is_active",
                                         addon.name,
                                         addon.price,
                                         id);
    return mapAddon(result[0]);
}

PostgresProductImageRepository::PostgresProductImageRepository(DbClientPtr db) : db_(std::move(db)) {}
std::optional<domain::menu::ProductImage> PostgresProductImageRepository::findMainByProductId(std::int64_t productId)
{
    const auto result = db_->execSqlSync(
        "select id, product_id, file_name, file_path, mime_type, file_size, is_main, cast(created_at as text) as created_at "
        "from product_images where product_id = $1 and is_main = true limit 1",
        productId);
    if (result.empty())
        return std::nullopt;
    return mapProductImage(result[0]);
}
domain::menu::ProductImage PostgresProductImageRepository::upsertMain(const domain::menu::ProductImage &image)
{
    const auto existing = findMainByProductId(image.productId);
    if (existing.has_value())
    {
        db_->execSqlSync("delete from product_images where product_id = $1 and id <> $2", image.productId, existing->id);
    }
    else
    {
        db_->execSqlSync("delete from product_images where product_id = $1", image.productId);
    }

    Result result;
    if (existing.has_value())
    {
        result = db_->execSqlSync(
            "update product_images "
            "set file_name = $1, file_path = $2, mime_type = $3, file_size = $4, is_main = true "
            "where id = $5 "
            "returning id, product_id, file_name, file_path, mime_type, file_size, is_main, cast(created_at as text) as created_at",
            image.fileName,
            image.filePath,
            image.mimeType,
            image.fileSize,
            existing->id);
    }
    else
    {
        result = db_->execSqlSync(
            "insert into product_images (product_id, file_name, file_path, mime_type, file_size, is_main) "
            "values ($1, $2, $3, $4, $5, true) "
            "returning id, product_id, file_name, file_path, mime_type, file_size, is_main, cast(created_at as text) as created_at",
            image.productId,
            image.fileName,
            image.filePath,
            image.mimeType,
            image.fileSize);
    }
    return mapProductImage(result[0]);
}
void PostgresProductImageRepository::deleteMain(std::int64_t productId)
{
    db_->execSqlSync("delete from product_images where product_id = $1 and is_main = true", productId);
}

PostgresProductRepository::PostgresProductRepository(DbClientPtr db,
                                                     domain::IAddonRepository &addonRepository,
                                                     domain::IProductImageRepository &productImageRepository)
    : db_(std::move(db)), addonRepository_(addonRepository), productImageRepository_(productImageRepository)
{
}
std::vector<domain::menu::Addon> PostgresProductRepository::loadAddons(std::int64_t productId)
{
    std::vector<domain::menu::Addon> addons;
    for (const auto &row : db_->execSqlSync("select a.id, a.name, a.price, a.is_active from addons a inner join product_addons pa on pa.addon_id = a.id where pa.product_id = $1 and a.is_active = true order by a.id asc", productId))
        addons.push_back(mapAddon(row));
    return addons;
}
std::optional<domain::menu::Product> PostgresProductRepository::findById(std::int64_t id)
{
    const auto result = db_->execSqlSync("select id, category_id, name, description, price, is_available, is_active from products where id = $1 limit 1", id);
    if (result.empty())
        return std::nullopt;
    auto product = mapProduct(result[0]);
    product.addons = loadAddons(product.id);
    product.image = productImageRepository_.findMainByProductId(product.id);
    return product;
}
std::vector<domain::menu::Product> PostgresProductRepository::listAll(bool onlyActive)
{
    std::vector<domain::menu::Product> products;
    const auto sql = onlyActive ? "select id, category_id, name, description, price, is_available, is_active from products where is_active = true order by id asc"
                                : "select id, category_id, name, description, price, is_available, is_active from products order by id asc";
    for (const auto &row : db_->execSqlSync(sql))
    {
        auto product = mapProduct(row);
        product.addons = loadAddons(product.id);
        product.image = productImageRepository_.findMainByProductId(product.id);
        products.push_back(product);
    }
    return products;
}
std::vector<domain::menu::Product> PostgresProductRepository::listPublicMenu()
{
    std::vector<domain::menu::Product> products;
    for (const auto &row : db_->execSqlSync("select id, category_id, name, description, price, is_available, is_active from products where is_active = true and is_available = true order by id asc"))
    {
        auto product = mapProduct(row);
        product.addons = loadAddons(product.id);
        product.image = productImageRepository_.findMainByProductId(product.id);
        products.push_back(product);
    }
    return products;
}
domain::menu::Product PostgresProductRepository::create(const domain::menu::Product &product)
{
    const auto result = db_->execSqlSync("insert into products (category_id, name, description, price, is_available, is_active) values ($1, $2, $3, $4, $5, $6) returning id, category_id, name, description, price, is_available, is_active",
                                         product.categoryId,
                                         product.name,
                                         product.description,
                                         product.price,
                                         product.isAvailable,
                                         product.isActive);
    return mapProduct(result[0]);
}
domain::menu::Product PostgresProductRepository::update(std::int64_t id, const domain::menu::Product &product)
{
    const auto result = db_->execSqlSync("update products set category_id = $1, name = $2, description = $3, price = $4, is_available = $5 where id = $6 returning id, category_id, name, description, price, is_available, is_active",
                                         product.categoryId,
                                         product.name,
                                         product.description,
                                         product.price,
                                         product.isAvailable,
                                         id);
    auto updated = mapProduct(result[0]);
    updated.addons = loadAddons(updated.id);
    updated.image = productImageRepository_.findMainByProductId(updated.id);
    return updated;
}
void PostgresProductRepository::markUnavailable(std::int64_t id) { db_->execSqlSync("update products set is_available = false where id = $1", id); }
void PostgresProductRepository::deactivate(std::int64_t id) { db_->execSqlSync("update products set is_active = false where id = $1", id); }
void PostgresProductRepository::assignAddon(std::int64_t productId, std::int64_t addonId)
{
    db_->execSqlSync("insert into product_addons (product_id, addon_id) values ($1, $2) on conflict do nothing", productId, addonId);
}

PostgresRestaurantTableRepository::PostgresRestaurantTableRepository(DbClientPtr db) : db_(std::move(db)) {}
domain::tables::RestaurantTable PostgresRestaurantTableRepository::create(const domain::tables::RestaurantTable &table)
{
    const auto result = db_->execSqlSync("insert into restaurant_tables (table_number, qr_token, is_active) values ($1, $2, $3) returning id, table_number, qr_token, is_active",
                                         table.tableNumber,
                                         table.qrToken,
                                         table.isActive);
    return mapTable(result[0]);
}
std::vector<domain::tables::RestaurantTable> PostgresRestaurantTableRepository::listActive()
{
    std::vector<domain::tables::RestaurantTable> tables;
    for (const auto &row : db_->execSqlSync("select id, table_number, qr_token, is_active from restaurant_tables where is_active = true order by table_number asc"))
        tables.push_back(mapTable(row));
    return tables;
}
std::optional<domain::tables::RestaurantTable> PostgresRestaurantTableRepository::findByQrToken(const std::string &qrToken)
{
    const auto result = db_->execSqlSync("select id, table_number, qr_token, is_active from restaurant_tables where qr_token = $1 limit 1", qrToken);
    if (result.empty())
        return std::nullopt;
    return mapTable(result[0]);
}
std::optional<domain::tables::RestaurantTable> PostgresRestaurantTableRepository::findById(std::int64_t id)
{
    const auto result = db_->execSqlSync("select id, table_number, qr_token, is_active from restaurant_tables where id = $1 limit 1", id);
    if (result.empty())
        return std::nullopt;
    return mapTable(result[0]);
}
domain::tables::RestaurantTable PostgresRestaurantTableRepository::updateQrToken(std::int64_t id, const std::string &qrToken)
{
    const auto result = db_->execSqlSync("update restaurant_tables set qr_token = $1 where id = $2 returning id, table_number, qr_token, is_active", qrToken, id);
    return mapTable(result[0]);
}
void PostgresRestaurantTableRepository::deactivate(std::int64_t id) { db_->execSqlSync("update restaurant_tables set is_active = false where id = $1", id); }

PostgresOrderRepository::PostgresOrderRepository(DbClientPtr db) : db_(std::move(db)) {}
std::vector<domain::orders::OrderItem> PostgresOrderRepository::loadItems(std::int64_t orderId)
{
    std::vector<domain::orders::OrderItem> items;
    for (const auto &row : db_->execSqlSync("select oi.id, oi.product_id, p.name as product_name, oi.quantity, oi.unit_price, oi.note as notes, oi.status from order_items oi inner join products p on p.id = oi.product_id where oi.order_id = $1 order by oi.id asc", orderId))
    {
        domain::orders::OrderItem item;
        item.id = row["id"].as<std::int64_t>();
        item.productId = row["product_id"].as<std::int64_t>();
        item.productName = row["product_name"].as<std::string>();
        item.quantity = row["quantity"].as<int>();
        item.unitPrice = row["unit_price"].as<double>();
        item.notes = nullableString(row, "notes");
        item.status = domain::orderItemStatusFromString(row["status"].as<std::string>());

        for (const auto &addonRow : db_->execSqlSync("select oia.addon_id, a.name, oia.price from order_item_addons oia inner join addons a on a.id = oia.addon_id where oia.order_item_id = $1 order by oia.addon_id asc", item.id))
        {
            item.addons.push_back({addonRow["addon_id"].as<std::int64_t>(), addonRow["name"].as<std::string>(), addonRow["price"].as<double>()});
        }
        items.push_back(item);
    }
    return items;
}
domain::orders::Order PostgresOrderRepository::create(const domain::orders::Order &order)
{
    domain::orders::Order created;
    {
        auto transaction = db_->newTransaction();
        const auto orderResult = transaction->execSqlSync("insert into orders (table_id, customer_name, status, total) values ($1, $2, $3, $4) returning id, table_id, customer_name, status, total, created_at",
                                                          order.tableId,
                                                          cleanText(order.customerName),
                                                          domain::toString(order.status),
                                                          order.total);
        created.id = orderResult[0]["id"].as<std::int64_t>();
        created.tableId = orderResult[0]["table_id"].as<std::int64_t>();
        created.tableNumber = order.tableNumber;
        created.customerName = orderResult[0]["customer_name"].as<std::string>();
        created.status = domain::orderStatusFromString(orderResult[0]["status"].as<std::string>());
        created.total = orderResult[0]["total"].as<double>();
        created.createdAt = orderResult[0]["created_at"].as<std::string>();

        for (const auto &item : order.items)
        {
            const auto itemResult = transaction->execSqlSync("insert into order_items (order_id, product_id, quantity, unit_price, note, status) values ($1, $2, $3, $4, $5, $6) returning id",
                                                             created.id,
                                                             item.productId,
                                                             item.quantity,
                                                             item.unitPrice,
                                                             cleanText(item.notes),
                                                             domain::toString(item.status));
            const auto itemId = itemResult[0]["id"].as<std::int64_t>();
            for (const auto &addon : item.addons)
            {
                transaction->execSqlSync("insert into order_item_addons (order_item_id, addon_id, price) values ($1, $2, $3)",
                                         itemId,
                                         addon.addonId,
                                         addon.price);
            }
        }
    }
    created.items = loadItems(created.id);
    return created;
}
std::optional<domain::orders::Order> PostgresOrderRepository::findById(std::int64_t id)
{
    const auto result = db_->execSqlSync("select o.id, o.table_id, rt.table_number, o.customer_name, o.status, o.total, o.created_at from orders o inner join restaurant_tables rt on rt.id = o.table_id where o.id = $1 limit 1", id);
    if (result.empty())
        return std::nullopt;
    auto order = mapOrderBase(result[0]);
    order.items = loadItems(order.id);
    return order;
}
std::vector<domain::orders::Order> PostgresOrderRepository::listKitchenActive()
{
    std::vector<domain::orders::Order> orders;
    for (const auto &row : db_->execSqlSync("select o.id, o.table_id, rt.table_number, o.customer_name, o.status, o.total, o.created_at from orders o inner join restaurant_tables rt on rt.id = o.table_id where o.status in ('PENDING', 'PREPARING', 'READY') order by o.created_at asc"))
    {
        auto order = mapOrderBase(row);
        order.items = loadItems(order.id);
        orders.push_back(order);
    }
    return orders;
}
std::vector<domain::orders::Order> PostgresOrderRepository::listHistory()
{
    std::vector<domain::orders::Order> orders;
    for (const auto &row : db_->execSqlSync("select o.id, o.table_id, rt.table_number, o.customer_name, o.status, o.total, o.created_at from orders o inner join restaurant_tables rt on rt.id = o.table_id order by o.created_at desc"))
    {
        auto order = mapOrderBase(row);
        order.items = loadItems(order.id);
        orders.push_back(order);
    }
    return orders;
}
std::vector<domain::orders::Order> PostgresOrderRepository::searchOrders(const std::string &customerName,
                                                                          const std::string &tableNumber,
                                                                          const std::string &status)
{
    std::vector<domain::orders::Order> orders;
    auto result = db_->execSqlSync(
        "select o.id, o.table_id, rt.table_number, o.customer_name, o.status, o.total, o.created_at "
        "from orders o inner join restaurant_tables rt on rt.id = o.table_id "
        "where ($1 = '' or lower(o.customer_name) like lower('%' || $1 || '%')) "
        "and ($2 = '' or cast(rt.table_number as text) = $2) "
        "and ($3 = '' or o.status = cast($3 as order_status)) "
        "order by o.created_at desc",
        customerName,
        tableNumber,
        status);
    for (const auto &row : result)
    {
        auto order = mapOrderBase(row);
        order.items = loadItems(order.id);
        orders.push_back(order);
    }
    return orders;
}
void PostgresOrderRepository::updateStatus(std::int64_t orderId, domain::OrderStatus status)
{
    db_->execSqlSync("update orders set status = $1 where id = $2", domain::toString(status), orderId);
}
void PostgresOrderRepository::updateOrderItemStatus(std::int64_t itemId, domain::OrderItemStatus status)
{
    db_->execSqlSync("update order_items set status = $1 where id = $2", domain::toString(status), itemId);
}
bool PostgresOrderRepository::allItemsReady(std::int64_t orderId)
{
    const auto result = db_->execSqlSync("select count(*) as pending_count from order_items where order_id = $1 and status <> 'READY'", orderId);
    return result[0]["pending_count"].as<std::int64_t>() == 0;
}

PostgresPaymentRepository::PostgresPaymentRepository(DbClientPtr db) : db_(std::move(db)) {}
std::optional<domain::payments::Payment> PostgresPaymentRepository::findByOrderId(std::int64_t orderId)
{
    const auto result = db_->execSqlSync("select id, order_id, amount, status, coalesce(cast(paid_at as text), '') as paid_at from payments where order_id = $1 limit 1", orderId);
    if (result.empty())
        return std::nullopt;
    return domain::payments::Payment{result[0]["id"].as<std::int64_t>(),
                                     result[0]["order_id"].as<std::int64_t>(),
                                     result[0]["amount"].as<double>(),
                                     domain::paymentStatusFromString(result[0]["status"].as<std::string>()),
                                     nullableString(result[0], "paid_at")};
}
domain::payments::Payment PostgresPaymentRepository::upsertPaid(std::int64_t orderId, double amount)
{
    const auto result = db_->execSqlSync(
        "insert into payments (order_id, amount, status, paid_at) values ($1, $2, 'PAID', now()) "
        "on conflict (order_id) do update set amount = excluded.amount, status = 'PAID', paid_at = now() "
        "returning id, order_id, amount, status, cast(paid_at as text) as paid_at",
        orderId,
        amount);
    return {result[0]["id"].as<std::int64_t>(),
            result[0]["order_id"].as<std::int64_t>(),
            result[0]["amount"].as<double>(),
            domain::paymentStatusFromString(result[0]["status"].as<std::string>()),
            result[0]["paid_at"].as<std::string>()};
}
std::vector<domain::payments::Payment> PostgresPaymentRepository::listAll()
{
    std::vector<domain::payments::Payment> payments;
    for (const auto &row : db_->execSqlSync("select id, order_id, amount, status, coalesce(cast(paid_at as text), '') as paid_at from payments order by id desc"))
    {
        payments.push_back({row["id"].as<std::int64_t>(),
                            row["order_id"].as<std::int64_t>(),
                            row["amount"].as<double>(),
                            domain::paymentStatusFromString(row["status"].as<std::string>()),
                            nullableString(row, "paid_at")});
    }
    return payments;
}
}  // namespace starcafe::infrastructure::repositories
