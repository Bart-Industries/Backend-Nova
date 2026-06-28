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

std::optional<std::int64_t> nullableInt64(const Row &row, const char *columnName)
{
    const auto field = row[columnName];
    if (field.isNull())
    {
        return std::nullopt;
    }
    return field.as<std::int64_t>();
}

std::string cleanText(std::string value)
{
    value.erase(std::remove(value.begin(), value.end(), '\0'), value.end());
    return value;
}

domain::businesses::Business mapBusiness(const Row &row)
{
    return {row["id"].as<std::int64_t>(),
            row["name"].as<std::string>(),
            row["slug"].as<std::string>(),
            nullableString(row, "logo_url"),
            nullableString(row, "primary_color"),
            row["is_active"].as<bool>(),
            row["created_at"].as<std::string>()};
}

domain::identity::User mapUser(const Row &row)
{
    domain::identity::User user;
    user.id = row["id"].as<std::int64_t>();
    user.name = row["name"].as<std::string>();
    user.email = row["email"].as<std::string>();
    user.passwordHash = row["password_hash"].as<std::string>();
    user.businessId = nullableInt64(row, "business_id");
    user.role = domain::userRoleFromString(row["role"].as<std::string>());
    user.isActive = row["is_active"].as<bool>();
    return user;
}

domain::menu::Category mapCategory(const Row &row)
{
    return {row["id"].as<std::int64_t>(), row["business_id"].as<std::int64_t>(), row["name"].as<std::string>(), nullableString(row, "description"), row["is_active"].as<bool>()};
}

domain::menu::Addon mapAddon(const Row &row)
{
    return {row["id"].as<std::int64_t>(), row["business_id"].as<std::int64_t>(), row["name"].as<std::string>(), row["price"].as<double>(), row["is_active"].as<bool>()};
}

domain::menu::Product mapProduct(const Row &row)
{
    return {row["id"].as<std::int64_t>(),
            row["business_id"].as<std::int64_t>(),
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
    return {row["id"].as<std::int64_t>(),
            row["business_id"].as<std::int64_t>(),
            nullableString(row, "business_name"),
            nullableString(row, "business_slug"),
            row["table_number"].as<int>(),
            row["qr_token"].as<std::string>(),
            row["is_active"].as<bool>()};
}

domain::orders::Order mapOrderBase(const Row &row)
{
    domain::orders::Order order;
    order.id = row["id"].as<std::int64_t>();
    order.businessId = row["business_id"].as<std::int64_t>();
    order.tableId = row["table_id"].as<std::int64_t>();
    order.tableNumber = row["table_number"].as<int>();
    order.customerName = row["customer_name"].as<std::string>();
    order.status = domain::orderStatusFromString(row["status"].as<std::string>());
    order.total = row["total"].as<double>();
    order.createdAt = row["created_at"].as<std::string>();
    order.updatedAt = row["updated_at"].as<std::string>();
    return order;
}

domain::payments::Payment mapPayment(const Row &row)
{
    return {row["id"].as<std::int64_t>(),
            row["business_id"].as<std::int64_t>(),
            row["order_id"].as<std::int64_t>(),
            row["amount"].as<double>(),
            domain::paymentStatusFromString(row["status"].as<std::string>()),
            nullableString(row, "paid_at")};
}
}  // namespace

PostgresBusinessRepository::PostgresBusinessRepository(DbClientPtr db) : db_(std::move(db)) {}
std::optional<domain::businesses::Business> PostgresBusinessRepository::findById(std::int64_t id)
{
    const auto result = db_->execSqlSync("select id, name, slug, coalesce(logo_url, '') as logo_url, coalesce(primary_color, '') as primary_color, is_active, cast(created_at as text) as created_at from businesses where id = $1 limit 1", id);
    if (result.empty()) return std::nullopt;
    return mapBusiness(result[0]);
}
std::optional<domain::businesses::Business> PostgresBusinessRepository::findBySlug(const std::string &slug)
{
    const auto result = db_->execSqlSync("select id, name, slug, coalesce(logo_url, '') as logo_url, coalesce(primary_color, '') as primary_color, is_active, cast(created_at as text) as created_at from businesses where slug = $1 limit 1", slug);
    if (result.empty()) return std::nullopt;
    return mapBusiness(result[0]);
}
std::optional<domain::businesses::Business> PostgresBusinessRepository::findSingleActive()
{
    const auto result = db_->execSqlSync("select id, name, slug, coalesce(logo_url, '') as logo_url, coalesce(primary_color, '') as primary_color, is_active, cast(created_at as text) as created_at from businesses where is_active = true order by id asc limit 2");
    if (result.size() != 1) return std::nullopt;
    return mapBusiness(result[0]);
}
std::vector<domain::businesses::Business> PostgresBusinessRepository::listAll(bool onlyActive)
{
    std::vector<domain::businesses::Business> businesses;
    const auto sql = onlyActive ? "select id, name, slug, coalesce(logo_url, '') as logo_url, coalesce(primary_color, '') as primary_color, is_active, cast(created_at as text) as created_at from businesses where is_active = true order by id asc"
                                : "select id, name, slug, coalesce(logo_url, '') as logo_url, coalesce(primary_color, '') as primary_color, is_active, cast(created_at as text) as created_at from businesses order by id asc";
    for (const auto &row : db_->execSqlSync(sql)) businesses.push_back(mapBusiness(row));
    return businesses;
}
domain::businesses::Business PostgresBusinessRepository::create(const domain::businesses::Business &business)
{
    const auto result = db_->execSqlSync("insert into businesses (name, slug, logo_url, primary_color, is_active) values ($1, $2, nullif($3, ''), nullif($4, ''), $5) returning id, name, slug, coalesce(logo_url, '') as logo_url, coalesce(primary_color, '') as primary_color, is_active, cast(created_at as text) as created_at",
                                         business.name, business.slug, business.logoUrl, business.primaryColor, business.isActive);
    return mapBusiness(result[0]);
}

PostgresUserRepository::PostgresUserRepository(DbClientPtr db) : db_(std::move(db)) {}
const std::string &PostgresUserRepository::passwordColumn()
{
    if (!passwordColumn_.empty()) return passwordColumn_;
    const auto result = db_->execSqlSync("select column_name from information_schema.columns where table_schema = 'public' and table_name = 'users' and column_name in ('password_hash', 'password')");
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
    const auto sql = "select id, business_id, name, email, " + passwordColumn() + " as password_hash, role, is_active from users where email = $1 limit 1";
    const auto result = db_->execSqlSync(sql, email);
    if (result.empty()) return std::nullopt;
    return mapUser(result[0]);
}
std::optional<domain::identity::User> PostgresUserRepository::findById(std::int64_t id)
{
    const auto sql = "select id, business_id, name, email, " + passwordColumn() + " as password_hash, role, is_active from users where id = $1 limit 1";
    const auto result = db_->execSqlSync(sql, id);
    if (result.empty()) return std::nullopt;
    return mapUser(result[0]);
}
std::vector<domain::identity::User> PostgresUserRepository::listActive(std::optional<std::int64_t> businessId)
{
    std::vector<domain::identity::User> users;
    const auto sql = businessId.has_value()
        ? "select id, business_id, name, email, " + passwordColumn() + " as password_hash, role, is_active from users where is_active = true and business_id = $1 order by id asc"
        : "select id, business_id, name, email, " + passwordColumn() + " as password_hash, role, is_active from users where is_active = true order by id asc";
    auto result = businessId.has_value() ? db_->execSqlSync(sql, *businessId) : db_->execSqlSync(sql);
    for (const auto &row : result) users.push_back(mapUser(row));
    return users;
}
std::int64_t PostgresUserRepository::countActiveByRole(domain::UserRole role, std::optional<std::int64_t> businessId)
{
    const auto sql = businessId.has_value()
        ? "select count(*) as count from users where role = $1 and is_active = true and business_id = $2"
        : "select count(*) as count from users where role = $1 and is_active = true and business_id is null";
    const auto result = businessId.has_value() ? db_->execSqlSync(sql, domain::toString(role), *businessId) : db_->execSqlSync(sql, domain::toString(role));
    return result[0]["count"].as<std::int64_t>();
}
domain::identity::User PostgresUserRepository::create(const domain::identity::User &user)
{
    const auto sql = "insert into users (business_id, name, email, " + passwordColumn() + ", role, is_active) values ($1, $2, $3, $4, $5, $6) returning id, business_id, name, email, " + passwordColumn() + " as password_hash, role, is_active";
    const auto result = db_->execSqlSync(sql, user.businessId.has_value() ? std::optional<std::int64_t>(*user.businessId) : std::optional<std::int64_t>{}, user.name, user.email, user.passwordHash, domain::toString(user.role), user.isActive);
    return mapUser(result[0]);
}
void PostgresUserRepository::deactivate(std::int64_t id) { db_->execSqlSync("update users set is_active = false where id = $1", id); }

PostgresCategoryRepository::PostgresCategoryRepository(DbClientPtr db) : db_(std::move(db)) {}
std::vector<domain::menu::Category> PostgresCategoryRepository::listAll(std::int64_t businessId, bool onlyActive)
{
    std::vector<domain::menu::Category> categories;
    const auto sql = onlyActive ? "select id, business_id, name, '' as description, is_active from categories where business_id = $1 and is_active = true order by id asc"
                                : "select id, business_id, name, '' as description, is_active from categories where business_id = $1 order by id asc";
    for (const auto &row : db_->execSqlSync(sql, businessId)) categories.push_back(mapCategory(row));
    return categories;
}
domain::menu::Category PostgresCategoryRepository::create(const domain::menu::Category &category)
{
    const auto result = db_->execSqlSync("insert into categories (business_id, name, is_active) values ($1, $2, $3) returning id, business_id, name, '' as description, is_active", category.businessId, category.name, category.isActive);
    return mapCategory(result[0]);
}
domain::menu::Category PostgresCategoryRepository::update(std::int64_t id, std::int64_t businessId, const domain::menu::Category &category)
{
    const auto result = db_->execSqlSync("update categories set name = $1 where id = $2 and business_id = $3 returning id, business_id, name, '' as description, is_active", category.name, id, businessId);
    if (result.empty()) throw domain::DomainError("Category not found");
    return mapCategory(result[0]);
}

PostgresAddonRepository::PostgresAddonRepository(DbClientPtr db) : db_(std::move(db)) {}
std::optional<domain::menu::Addon> PostgresAddonRepository::findById(std::int64_t id)
{
    const auto result = db_->execSqlSync("select id, business_id, name, price, is_active from addons where id = $1 limit 1", id);
    if (result.empty()) return std::nullopt;
    return mapAddon(result[0]);
}
std::vector<domain::menu::Addon> PostgresAddonRepository::listAll(std::int64_t businessId, bool onlyActive)
{
    std::vector<domain::menu::Addon> addons;
    const auto sql = onlyActive ? "select id, business_id, name, price, is_active from addons where business_id = $1 and is_active = true order by id asc"
                                : "select id, business_id, name, price, is_active from addons where business_id = $1 order by id asc";
    for (const auto &row : db_->execSqlSync(sql, businessId)) addons.push_back(mapAddon(row));
    return addons;
}
domain::menu::Addon PostgresAddonRepository::create(const domain::menu::Addon &addon)
{
    const auto result = db_->execSqlSync("insert into addons (business_id, name, price, is_active) values ($1, $2, $3, $4) returning id, business_id, name, price, is_active", addon.businessId, addon.name, addon.price, addon.isActive);
    return mapAddon(result[0]);
}
domain::menu::Addon PostgresAddonRepository::update(std::int64_t id, std::int64_t businessId, const domain::menu::Addon &addon)
{
    const auto result = db_->execSqlSync("update addons set name = $1, price = $2 where id = $3 and business_id = $4 returning id, business_id, name, price, is_active", addon.name, addon.price, id, businessId);
    if (result.empty()) throw domain::DomainError("Addon not found");
    return mapAddon(result[0]);
}

PostgresProductImageRepository::PostgresProductImageRepository(DbClientPtr db) : db_(std::move(db)) {}
std::optional<domain::menu::ProductImage> PostgresProductImageRepository::findMainByProductId(std::int64_t productId)
{
    const auto result = db_->execSqlSync("select id, product_id, file_name, file_path, mime_type, file_size, is_main, cast(created_at as text) as created_at from product_images where product_id = $1 and is_main = true limit 1", productId);
    if (result.empty()) return std::nullopt;
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
        result = db_->execSqlSync("update product_images set file_name = $1, file_path = $2, mime_type = $3, file_size = $4, is_main = true where id = $5 returning id, product_id, file_name, file_path, mime_type, file_size, is_main, cast(created_at as text) as created_at", image.fileName, image.filePath, image.mimeType, image.fileSize, existing->id);
    }
    else
    {
        result = db_->execSqlSync("insert into product_images (product_id, file_name, file_path, mime_type, file_size, is_main) values ($1, $2, $3, $4, $5, true) returning id, product_id, file_name, file_path, mime_type, file_size, is_main, cast(created_at as text) as created_at", image.productId, image.fileName, image.filePath, image.mimeType, image.fileSize);
    }
    return mapProductImage(result[0]);
}
void PostgresProductImageRepository::deleteMain(std::int64_t productId) { db_->execSqlSync("delete from product_images where product_id = $1 and is_main = true", productId); }

PostgresProductRepository::PostgresProductRepository(DbClientPtr db, domain::IAddonRepository &addonRepository, domain::IProductImageRepository &productImageRepository)
    : db_(std::move(db)), addonRepository_(addonRepository), productImageRepository_(productImageRepository)
{
}
std::vector<domain::menu::Addon> PostgresProductRepository::loadAddons(std::int64_t productId)
{
    std::vector<domain::menu::Addon> addons;
    for (const auto &row : db_->execSqlSync("select a.id, a.business_id, a.name, a.price, a.is_active from addons a inner join product_addons pa on pa.addon_id = a.id where pa.product_id = $1 and a.is_active = true order by a.id asc", productId)) addons.push_back(mapAddon(row));
    return addons;
}
std::optional<domain::menu::Product> PostgresProductRepository::findById(std::int64_t id)
{
    const auto result = db_->execSqlSync("select id, business_id, category_id, name, description, price, is_available, is_active from products where id = $1 limit 1", id);
    if (result.empty()) return std::nullopt;
    auto product = mapProduct(result[0]);
    product.addons = loadAddons(product.id);
    product.image = productImageRepository_.findMainByProductId(product.id);
    return product;
}
std::vector<domain::menu::Product> PostgresProductRepository::listAll(std::int64_t businessId, bool onlyActive)
{
    std::vector<domain::menu::Product> products;
    const auto sql = onlyActive ? "select id, business_id, category_id, name, description, price, is_available, is_active from products where business_id = $1 and is_active = true order by id asc"
                                : "select id, business_id, category_id, name, description, price, is_available, is_active from products where business_id = $1 order by id asc";
    for (const auto &row : db_->execSqlSync(sql, businessId))
    {
        auto product = mapProduct(row);
        product.addons = loadAddons(product.id);
        product.image = productImageRepository_.findMainByProductId(product.id);
        products.push_back(product);
    }
    return products;
}
std::vector<domain::menu::Product> PostgresProductRepository::listPublicMenu(std::int64_t businessId)
{
    std::vector<domain::menu::Product> products;
    for (const auto &row : db_->execSqlSync("select id, business_id, category_id, name, description, price, is_available, is_active from products where business_id = $1 and is_active = true and is_available = true order by id asc", businessId))
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
    const auto result = db_->execSqlSync("insert into products (business_id, category_id, name, description, price, is_available, is_active) values ($1, $2, $3, $4, $5, $6, $7) returning id, business_id, category_id, name, description, price, is_available, is_active", product.businessId, product.categoryId, product.name, product.description, product.price, product.isAvailable, product.isActive);
    return mapProduct(result[0]);
}
domain::menu::Product PostgresProductRepository::update(std::int64_t id, const domain::menu::Product &product)
{
    const auto result = db_->execSqlSync("update products set category_id = $1, name = $2, description = $3, price = $4, is_available = $5 where id = $6 and business_id = $7 returning id, business_id, category_id, name, description, price, is_available, is_active", product.categoryId, product.name, product.description, product.price, product.isAvailable, id, product.businessId);
    if (result.empty()) throw domain::DomainError("Product not found");
    auto updated = mapProduct(result[0]);
    updated.addons = loadAddons(updated.id);
    updated.image = productImageRepository_.findMainByProductId(updated.id);
    return updated;
}
void PostgresProductRepository::activate(std::int64_t id) { db_->execSqlSync("update products set is_active = true where id = $1", id); }
void PostgresProductRepository::markUnavailable(std::int64_t id) { db_->execSqlSync("update products set is_available = false where id = $1", id); }
void PostgresProductRepository::deactivate(std::int64_t id) { db_->execSqlSync("update products set is_active = false where id = $1", id); }
void PostgresProductRepository::assignAddon(std::int64_t productId, std::int64_t addonId)
{
    db_->execSqlSync("insert into product_addons (product_id, addon_id) values ($1, $2) on conflict do nothing", productId, addonId);
}

PostgresRestaurantTableRepository::PostgresRestaurantTableRepository(DbClientPtr db) : db_(std::move(db)) {}
domain::tables::RestaurantTable PostgresRestaurantTableRepository::create(const domain::tables::RestaurantTable &table)
{
    const auto result = db_->execSqlSync("insert into restaurant_tables (business_id, table_number, qr_token, is_active) values ($1, $2, $3, $4) returning id, business_id, table_number, qr_token, is_active", table.businessId, table.tableNumber, table.qrToken, table.isActive);
    auto created = mapTable(result[0]);
    const auto business = db_->execSqlSync("select name as business_name, slug as business_slug from businesses where id = $1 limit 1", created.businessId);
    if (!business.empty())
    {
        created.businessName = business[0]["business_name"].as<std::string>();
        created.businessSlug = business[0]["business_slug"].as<std::string>();
    }
    return created;
}
std::vector<domain::tables::RestaurantTable> PostgresRestaurantTableRepository::listActive(std::int64_t businessId)
{
    std::vector<domain::tables::RestaurantTable> tables;
    for (const auto &row : db_->execSqlSync("select rt.id, rt.business_id, b.name as business_name, b.slug as business_slug, rt.table_number, rt.qr_token, rt.is_active from restaurant_tables rt inner join businesses b on b.id = rt.business_id where rt.business_id = $1 and rt.is_active = true order by rt.table_number asc", businessId)) tables.push_back(mapTable(row));
    return tables;
}
std::optional<domain::tables::RestaurantTable> PostgresRestaurantTableRepository::findByQrToken(const std::string &qrToken)
{
    const auto result = db_->execSqlSync("select rt.id, rt.business_id, b.name as business_name, b.slug as business_slug, rt.table_number, rt.qr_token, rt.is_active from restaurant_tables rt inner join businesses b on b.id = rt.business_id where rt.qr_token = $1 limit 1", qrToken);
    if (result.empty()) return std::nullopt;
    return mapTable(result[0]);
}
std::optional<domain::tables::RestaurantTable> PostgresRestaurantTableRepository::findById(std::int64_t id)
{
    const auto result = db_->execSqlSync("select rt.id, rt.business_id, b.name as business_name, b.slug as business_slug, rt.table_number, rt.qr_token, rt.is_active from restaurant_tables rt inner join businesses b on b.id = rt.business_id where rt.id = $1 limit 1", id);
    if (result.empty()) return std::nullopt;
    return mapTable(result[0]);
}
domain::tables::RestaurantTable PostgresRestaurantTableRepository::updateQrToken(std::int64_t id, const std::string &qrToken)
{
    const auto result = db_->execSqlSync("update restaurant_tables set qr_token = $1 where id = $2 returning id, business_id, table_number, qr_token, is_active", qrToken, id);
    if (result.empty()) throw domain::DomainError("Table not found");
    auto updated = mapTable(result[0]);
    const auto business = db_->execSqlSync("select name as business_name, slug as business_slug from businesses where id = $1 limit 1", updated.businessId);
    if (!business.empty())
    {
        updated.businessName = business[0]["business_name"].as<std::string>();
        updated.businessSlug = business[0]["business_slug"].as<std::string>();
    }
    return updated;
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
        const auto orderResult = transaction->execSqlSync("insert into orders (business_id, table_id, customer_name, status, total) values ($1, $2, $3, $4, $5) returning id, business_id, table_id, customer_name, status, total, cast(created_at as text) as created_at, cast(updated_at as text) as updated_at", order.businessId, order.tableId, cleanText(order.customerName), domain::toString(order.status), order.total);
        created = mapOrderBase(orderResult[0]);
        created.tableNumber = order.tableNumber;
        for (const auto &item : order.items)
        {
            const auto itemResult = transaction->execSqlSync("insert into order_items (order_id, product_id, quantity, unit_price, note, status) values ($1, $2, $3, $4, $5, $6) returning id", created.id, item.productId, item.quantity, item.unitPrice, cleanText(item.notes), domain::toString(item.status));
            const auto itemId = itemResult[0]["id"].as<std::int64_t>();
            for (const auto &addon : item.addons)
            {
                transaction->execSqlSync("insert into order_item_addons (order_item_id, addon_id, price) values ($1, $2, $3)", itemId, addon.addonId, addon.price);
            }
        }
    }
    created.items = loadItems(created.id);
    return created;
}
std::optional<domain::orders::Order> PostgresOrderRepository::findById(std::int64_t id)
{
    const auto result = db_->execSqlSync("select o.id, o.business_id, o.table_id, rt.table_number, o.customer_name, o.status, o.total, cast(o.created_at as text) as created_at, cast(o.updated_at as text) as updated_at from orders o inner join restaurant_tables rt on rt.id = o.table_id where o.id = $1 limit 1", id);
    if (result.empty()) return std::nullopt;
    auto order = mapOrderBase(result[0]);
    order.items = loadItems(order.id);
    return order;
}
std::vector<domain::orders::Order> PostgresOrderRepository::listKitchenActive(std::int64_t businessId)
{
    std::vector<domain::orders::Order> orders;
    for (const auto &row : db_->execSqlSync("select o.id, o.business_id, o.table_id, rt.table_number, o.customer_name, o.status, o.total, cast(o.created_at as text) as created_at, cast(o.updated_at as text) as updated_at from orders o inner join restaurant_tables rt on rt.id = o.table_id where o.business_id = $1 and o.status in ('PENDING', 'PREPARING', 'READY') order by o.updated_at desc, o.created_at desc", businessId))
    {
        auto order = mapOrderBase(row);
        order.items = loadItems(order.id);
        orders.push_back(order);
    }
    return orders;
}
std::vector<domain::orders::Order> PostgresOrderRepository::listHistory(std::int64_t businessId)
{
    std::vector<domain::orders::Order> orders;
    for (const auto &row : db_->execSqlSync("select o.id, o.business_id, o.table_id, rt.table_number, o.customer_name, o.status, o.total, cast(o.created_at as text) as created_at, cast(o.updated_at as text) as updated_at from orders o inner join restaurant_tables rt on rt.id = o.table_id where o.business_id = $1 order by o.updated_at desc, o.created_at desc", businessId))
    {
        auto order = mapOrderBase(row);
        order.items = loadItems(order.id);
        orders.push_back(order);
    }
    return orders;
}
std::vector<domain::orders::Order> PostgresOrderRepository::listActiveByTableId(std::int64_t tableId)
{
    std::vector<domain::orders::Order> orders;
    for (const auto &row : db_->execSqlSync("select o.id, o.business_id, o.table_id, rt.table_number, o.customer_name, o.status, o.total, cast(o.created_at as text) as created_at, cast(o.updated_at as text) as updated_at from orders o inner join restaurant_tables rt on rt.id = o.table_id where o.table_id = $1 and o.status in ('PENDING', 'PREPARING', 'READY') order by o.updated_at desc, o.created_at desc", tableId))
    {
        auto order = mapOrderBase(row);
        order.items = loadItems(order.id);
        orders.push_back(order);
    }
    return orders;
}
std::int64_t PostgresOrderRepository::countActiveByTableId(std::int64_t tableId)
{
    const auto result = db_->execSqlSync("select count(*) as count from orders where table_id = $1 and status in ('PENDING', 'PREPARING', 'READY')", tableId);
    return result[0]["count"].as<std::int64_t>();
}
std::vector<domain::orders::Order> PostgresOrderRepository::searchOrders(std::int64_t businessId, const std::string &customerName, const std::string &tableNumber, const std::string &status)
{
    std::vector<domain::orders::Order> orders;
    auto result = db_->execSqlSync("select o.id, o.business_id, o.table_id, rt.table_number, o.customer_name, o.status, o.total, cast(o.created_at as text) as created_at, cast(o.updated_at as text) as updated_at from orders o inner join restaurant_tables rt on rt.id = o.table_id where o.business_id = $1 and ($2 = '' or lower(o.customer_name) like lower('%' || $2 || '%')) and ($3 = '' or cast(rt.table_number as text) = $3) and ($4 = '' or o.status = cast($4 as order_status)) order by o.updated_at desc, o.created_at desc", businessId, customerName, tableNumber, status);
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
    const auto result = db_->execSqlSync("select id, business_id, order_id, amount, status, coalesce(cast(paid_at as text), '') as paid_at from payments where order_id = $1 limit 1", orderId);
    if (result.empty()) return std::nullopt;
    return mapPayment(result[0]);
}
domain::payments::Payment PostgresPaymentRepository::upsertPaid(std::int64_t businessId, std::int64_t orderId, double amount)
{
    const auto result = db_->execSqlSync("insert into payments (business_id, order_id, amount, status, paid_at) values ($1, $2, $3, 'PAID', now()) on conflict (order_id) do update set business_id = excluded.business_id, amount = excluded.amount, status = 'PAID', paid_at = now() returning id, business_id, order_id, amount, status, cast(paid_at as text) as paid_at", businessId, orderId, amount);
    return mapPayment(result[0]);
}
std::vector<domain::payments::Payment> PostgresPaymentRepository::listAll(std::int64_t businessId)
{
    std::vector<domain::payments::Payment> payments;
    for (const auto &row : db_->execSqlSync("select id, business_id, order_id, amount, status, coalesce(cast(paid_at as text), '') as paid_at from payments where business_id = $1 order by id desc", businessId))
    {
        payments.push_back(mapPayment(row));
    }
    return payments;
}
}  // namespace starcafe::infrastructure::repositories
