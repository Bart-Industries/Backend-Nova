#include "interfaces/rest/api_controller.h"

#include "application/common/dtos.h"
#include "domain/businesses/theme_catalog.h"
#include "domain/common/enums.h"
#include "domain/common/errors.h"
#include "infrastructure/security/auth_filters.h"
#include "interfaces/rest/service_registry.h"

#include <algorithm>
#include <json/json.h>
#include <optional>

namespace starcafe::interfaces::rest
{
namespace
{
ServiceRegistry registry;
Json::Value orderToJson(const domain::orders::Order &order);

std::string sanitizeText(std::string value)
{
    value.erase(std::remove(value.begin(), value.end(), '\0'), value.end());
    return value;
}

bool isTruthy(const std::string &value)
{
    return value == "1" || value == "true" || value == "TRUE" || value == "yes" || value == "on";
}

Json::Value successResponse(const Json::Value &data)
{
    Json::Value body;
    body["success"] = true;
    body["data"] = data;
    return body;
}

drogon::HttpResponsePtr jsonResponse(const Json::Value &value, drogon::HttpStatusCode code = drogon::k200OK)
{
    auto response = drogon::HttpResponse::newHttpJsonResponse(value);
    response->setStatusCode(code);
    return response;
}

drogon::HttpResponsePtr errorResponse(const std::string &message, drogon::HttpStatusCode code = drogon::k400BadRequest)
{
    Json::Value body;
    body["success"] = false;
    body["message"] = message;
    return jsonResponse(body, code);
}

template <typename Func> void executeSafely(std::function<void(const drogon::HttpResponsePtr &)> &callback, Func &&func)
{
    try
    {
        callback(func());
    }
    catch (const domain::DomainError &error)
    {
        callback(errorResponse(error.what()));
    }
    catch (const std::exception &error)
    {
        callback(errorResponse(error.what(), drogon::k500InternalServerError));
    }
}

const Json::Value &body(const drogon::HttpRequestPtr &req)
{
    const auto json = req->getJsonObject();
    if (!json)
    {
        throw domain::DomainError("Request body must be valid JSON");
    }
    return *json;
}

auto requireClaims(const drogon::HttpRequestPtr &req)
{
    const auto claims = infrastructure::security::parseBearerClaims(req);
    if (!claims.has_value())
    {
        throw domain::DomainError("Invalid token");
    }
    return *claims;
}

void ensureSuperAdmin(const infrastructure::security::JwtClaims &claims)
{
    if (claims.role != "SUPER_ADMIN")
    {
        throw domain::DomainError("SUPER_ADMIN role required");
    }
}

std::int64_t resolveBusinessId(const drogon::HttpRequestPtr &req, const Json::Value *json = nullptr)
{
    const auto claims = requireClaims(req);
    if (claims.businessId.has_value())
    {
        return *claims.businessId;
    }
    if (claims.role != "SUPER_ADMIN")
    {
        throw domain::DomainError("Business context is required");
    }
    if (json != nullptr && json->isMember("businessId") && !(*json)["businessId"].isNull())
    {
        return (*json)["businessId"].asInt64();
    }
    const auto parameter = req->getParameter("businessId");
    if (!parameter.empty())
    {
        return std::stoll(parameter);
    }
    throw domain::DomainError("businessId is required for SUPER_ADMIN requests");
}

Json::Value businessToJson(const domain::businesses::Business &business)
{
    Json::Value value;
    value["id"] = Json::Int64(business.id);
    value["name"] = business.name;
    value["slug"] = business.slug;
    value["logoUrl"] = business.logoUrl;
    value["primaryColor"] = business.primaryColor;
    value["isActive"] = business.isActive;
    value["createdAt"] = business.createdAt;
    if (const auto themeKey = domain::businesses::themeKeyFromColor(business.primaryColor); themeKey.has_value())
    {
        value["themeKey"] = std::string(*themeKey);
    }
    else
    {
        value["themeKey"] = Json::nullValue;
    }
    return value;
}

Json::Value userToJson(const domain::identity::User &user)
{
    Json::Value value;
    value["id"] = Json::Int64(user.id);
    if (user.businessId.has_value())
    {
        value["businessId"] = Json::Int64(*user.businessId);
    }
    else
    {
        value["businessId"] = Json::nullValue;
    }
    value["name"] = user.name;
    value["email"] = user.email;
    value["role"] = domain::toString(user.role);
    value["isActive"] = user.isActive;
    return value;
}

Json::Value tableToJson(const domain::tables::RestaurantTable &table)
{
    Json::Value value;
    value["id"] = Json::Int64(table.id);
    value["businessId"] = Json::Int64(table.businessId);
    value["businessName"] = table.businessName;
    value["businessSlug"] = table.businessSlug;
    value["tableNumber"] = table.tableNumber;
    value["qrToken"] = table.qrToken;
    value["qrUrl"] = services().frontendBaseUrl + "/c/" + table.businessSlug + "/mesa/" + table.qrToken;
    value["isActive"] = table.isActive;
    return value;
}

Json::Value categoryToJson(const domain::menu::Category &category)
{
    Json::Value value;
    value["id"] = Json::Int64(category.id);
    value["businessId"] = Json::Int64(category.businessId);
    value["name"] = category.name;
    value["description"] = category.description;
    value["isActive"] = category.isActive;
    return value;
}

Json::Value addonToJson(const domain::menu::Addon &addon)
{
    Json::Value value;
    value["id"] = Json::Int64(addon.id);
    value["businessId"] = Json::Int64(addon.businessId);
    value["name"] = addon.name;
    value["price"] = addon.price;
    value["isActive"] = addon.isActive;
    return value;
}

Json::Value productToJson(const domain::menu::Product &product)
{
    Json::Value value;
    value["id"] = Json::Int64(product.id);
    value["businessId"] = Json::Int64(product.businessId);
    value["categoryId"] = Json::Int64(product.categoryId);
    value["name"] = product.name;
    value["description"] = product.description;
    value["price"] = product.price;
    value["isAvailable"] = product.isAvailable;
    value["isActive"] = product.isActive;
    for (const auto &addon : product.addons)
    {
        value["addons"].append(addonToJson(addon));
    }
    if (product.image.has_value())
    {
        Json::Value image;
        image["id"] = Json::Int64(product.image->id);
        image["file_name"] = product.image->fileName;
        image["mime_type"] = product.image->mimeType;
        image["file_size"] = Json::Int64(product.image->fileSize);
        image["url"] = services().fileStorageService->publicUrl(product.image->filePath, product.image->mimeType);
        value["image"] = image;
    }
    else
    {
        value["image"] = Json::nullValue;
    }
    return value;
}

Json::Value orderToJson(const domain::orders::Order &order)
{
    Json::Value value;
    value["id"] = Json::Int64(order.id);
    value["businessId"] = Json::Int64(order.businessId);
    value["tableId"] = Json::Int64(order.tableId);
    value["tableNumber"] = order.tableNumber;
    value["customerName"] = order.customerName;
    value["status"] = domain::toString(order.status);
    value["total"] = order.total;
    value["createdAt"] = order.createdAt;
    value["updatedAt"] = order.updatedAt;
    for (const auto &item : order.items)
    {
        Json::Value jsonItem;
        jsonItem["id"] = Json::Int64(item.id);
        jsonItem["productId"] = Json::Int64(item.productId);
        jsonItem["productName"] = item.productName;
        jsonItem["quantity"] = item.quantity;
        jsonItem["unitPrice"] = item.unitPrice;
        jsonItem["notes"] = item.notes;
        jsonItem["status"] = domain::toString(item.status);
        for (const auto &addon : item.addons)
        {
            Json::Value jsonAddon;
            jsonAddon["addonId"] = Json::Int64(addon.addonId);
            jsonAddon["name"] = addon.name;
            jsonAddon["price"] = addon.price;
            jsonItem["addons"].append(jsonAddon);
        }
        value["items"].append(jsonItem);
    }
    return value;
}

Json::Value paymentToJson(const domain::payments::Payment &payment)
{
    Json::Value value;
    value["id"] = Json::Int64(payment.id);
    value["businessId"] = Json::Int64(payment.businessId);
    value["orderId"] = Json::Int64(payment.orderId);
    value["amount"] = payment.amount;
    value["status"] = domain::toString(payment.status);
    value["paidAt"] = payment.paidAt;
    return value;
}

Json::Value publicTableSessionToJson(const application::orders::PublicTableSession &session)
{
    Json::Value value;
    value["business"] = businessToJson(session.business);
    value["table"] = tableToJson(session.table);
    value["activeOrdersCount"] = Json::Int64(session.activeOrdersCount);
    value["remainingSlots"] = Json::Int64(session.remainingSlots);
    value["canCreateMoreOrders"] = session.canCreateMoreOrders;
    for (const auto &order : session.activeOrders)
    {
        value["activeOrders"].append(orderToJson(order));
    }
    return value;
}
}  // namespace

ServiceRegistry &services() { return registry; }

void ApiController::registerUser(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    executeSafely(callback, [&]() {
        const auto &json = body(req);
        const auto claims = infrastructure::security::parseBearerClaims(req);
        const bool bootstrapMode = registry.userRepository->listActive().empty();

        if (!bootstrapMode)
        {
            if (!claims.has_value())
            {
                throw domain::DomainError("Authentication is required to register users");
            }
            ensureSuperAdmin(*claims);
        }

        application::RegisterUserCommand command;
        command.name = sanitizeText(json["name"].asString());
        command.email = sanitizeText(json["email"].asString());
        command.password = sanitizeText(json["password"].asString());
        command.role = domain::userRoleFromString(json["role"].asString());
        if (json.isMember("businessId") && !json["businessId"].isNull())
        {
            command.businessId = json["businessId"].asInt64();
        }

        if (bootstrapMode)
        {
            if (command.role != domain::UserRole::SUPER_ADMIN || command.businessId.has_value())
            {
                throw domain::DomainError("Bootstrap registration only allows a SUPER_ADMIN without businessId");
            }
        }

        return jsonResponse(successResponse(userToJson(registry.registerUser->execute(command))), drogon::k201Created);
    });
}

void ApiController::login(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    executeSafely(callback, [&]() {
        const auto &json = body(req);
        const auto auth = registry.login->execute({sanitizeText(json["email"].asString()), sanitizeText(json["password"].asString())});
        Json::Value data;
        data["token"] = auth.token;
        data["userId"] = Json::Int64(auth.userId);
        if (auth.businessId.has_value())
        {
            data["businessId"] = Json::Int64(*auth.businessId);
        }
        else
        {
            data["businessId"] = Json::nullValue;
        }
        data["role"] = auth.role;
        data["name"] = auth.name;
        data["email"] = auth.email;
        return jsonResponse(successResponse(data));
    });
}

void ApiController::me(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    executeSafely(callback, [&]() { return jsonResponse(successResponse(userToJson(registry.getCurrentUser->execute(requireClaims(req).userId)))); });
}

void ApiController::listBusinesses(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    executeSafely(callback, [&]() {
        ensureSuperAdmin(requireClaims(req));
        Json::Value data(Json::arrayValue);
        const bool includeInactive = isTruthy(req->getParameter("includeInactive"));
        for (const auto &business : registry.listBusinesses->execute(!includeInactive))
        {
            data.append(businessToJson(business));
        }
        return jsonResponse(successResponse(data));
    });
}

void ApiController::createBusiness(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    executeSafely(callback, [&]() {
        ensureSuperAdmin(requireClaims(req));
        const auto &json = body(req);
        application::CreateBusinessCommand command;
        command.name = sanitizeText(json["name"].asString());
        command.slug = sanitizeText(json["slug"].asString());
        command.logoUrl = sanitizeText(json.get("logoUrl", "").asString());
        command.primaryColor = sanitizeText(json.get("primaryColor", "").asString());
        return jsonResponse(successResponse(businessToJson(registry.createBusiness->execute(command))), drogon::k201Created);
    });
}

void ApiController::getTableByQr(const drogon::HttpRequestPtr &, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string qrToken)
{
    executeSafely(callback, [&]() { return jsonResponse(successResponse(tableToJson(registry.getTableByQrToken->execute(qrToken)))); });
}

void ApiController::getPublicTableSession(const drogon::HttpRequestPtr &, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string qrToken)
{
    executeSafely(callback, [&]() { return jsonResponse(successResponse(publicTableSessionToJson(registry.getPublicTableSession->execute(qrToken)))); });
}

void ApiController::getPublicMenu(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    executeSafely(callback, [&]() {
        Json::Value data(Json::arrayValue);
        for (const auto &product : registry.getPublicMenu->execute(req->getParameter("businessSlug")))
        {
            data.append(productToJson(product));
        }
        return jsonResponse(successResponse(data));
    });
}

void ApiController::createOrderFromTable(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string qrToken)
{
    executeSafely(callback, [&]() {
        const auto &json = body(req);
        application::CreateOrderFromTableCommand command;
        command.qrToken = qrToken;
        command.customerName = sanitizeText(json["customerName"].asString());
        for (const auto &item : json["items"])
        {
            application::CreateOrderItemCommand itemCommand;
            itemCommand.productId = item["productId"].asInt64();
            itemCommand.quantity = item["quantity"].asInt();
            itemCommand.notes = sanitizeText(item.get("notes", "").asString());
            for (const auto &addonId : item["addonIds"])
            {
                itemCommand.addonIds.push_back(addonId.asInt64());
            }
            command.items.push_back(itemCommand);
        }
        return jsonResponse(successResponse(orderToJson(registry.createOrderFromTable->execute(command))), drogon::k201Created);
    });
}

void ApiController::getOrderStatus(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string orderId)
{
    executeSafely(callback, [&]() {
        const auto qrToken = sanitizeText(req->getParameter("qrToken"));
        if (qrToken.empty())
        {
            throw domain::DomainError("qrToken is required");
        }
        return jsonResponse(successResponse(orderToJson(registry.getOrderStatus->execute(std::stoll(orderId), qrToken))));
    });
}

void ApiController::getKitchenOrders(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    executeSafely(callback, [&]() {
        Json::Value data(Json::arrayValue);
        for (const auto &order : registry.getKitchenOrders->execute(resolveBusinessId(req)))
        {
            data.append(orderToJson(order));
        }
        return jsonResponse(successResponse(data));
    });
}

void ApiController::startPreparingOrder(const drogon::HttpRequestPtr &req,
                                        std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                                        std::string orderId)
{
    executeSafely(callback, [&]() {
        registry.startPreparingOrder->execute(resolveBusinessId(req), std::stoll(orderId));
        return jsonResponse(successResponse(Json::Value("Order moved to PREPARING")));
    });
}

void ApiController::markOrderItemReady(const drogon::HttpRequestPtr &req,
                                       std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                                       std::string itemId)
{
    executeSafely(callback, [&]() {
        registry.markOrderItemReady->execute(resolveBusinessId(req), std::stoll(itemId));
        return jsonResponse(successResponse(Json::Value("Order item marked as READY")));
    });
}

void ApiController::markOrderReady(const drogon::HttpRequestPtr &req,
                                   std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                                   std::string orderId)
{
    executeSafely(callback, [&]() {
        registry.markOrderReady->execute(resolveBusinessId(req), std::stoll(orderId));
        return jsonResponse(successResponse(Json::Value("Order marked as READY")));
    });
}

void ApiController::getOrdersHistory(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    executeSafely(callback, [&]() {
        Json::Value data(Json::arrayValue);
        for (const auto &order : registry.getOrdersHistory->execute(resolveBusinessId(req)))
        {
            data.append(orderToJson(order));
        }
        return jsonResponse(successResponse(data));
    });
}

void ApiController::listUsers(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    executeSafely(callback, [&]() {
        const auto claims = requireClaims(req);
        std::optional<std::int64_t> businessId;
        if (claims.businessId.has_value())
        {
            businessId = claims.businessId;
        }
        else if (!req->getParameter("businessId").empty())
        {
            businessId = std::stoll(req->getParameter("businessId"));
        }

        Json::Value data(Json::arrayValue);
        for (const auto &user : registry.userRepository->listActive(businessId))
        {
            data.append(userToJson(user));
        }
        return jsonResponse(successResponse(data));
    });
}

void ApiController::createAdminUser(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    executeSafely(callback, [&]() {
        const auto claims = requireClaims(req);
        const auto &json = body(req);
        application::RegisterUserCommand command;
        command.name = sanitizeText(json["name"].asString());
        command.email = sanitizeText(json["email"].asString());
        command.password = sanitizeText(json["password"].asString());
        command.role = domain::userRoleFromString(json["role"].asString());

        if (command.role == domain::UserRole::SUPER_ADMIN)
        {
            throw domain::DomainError("SUPER_ADMIN users must be created through the global register flow");
        }

        if (claims.businessId.has_value())
        {
            command.businessId = claims.businessId;
        }
        else if (json.isMember("businessId") && !json["businessId"].isNull())
        {
            command.businessId = json["businessId"].asInt64();
        }
        else
        {
            throw domain::DomainError("businessId is required");
        }

        return jsonResponse(successResponse(userToJson(registry.registerUser->execute(command))), drogon::k201Created);
    });
}

void ApiController::deactivateUser(const drogon::HttpRequestPtr &req,
                                   std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                                   std::string id)
{
    executeSafely(callback, [&]() {
        const auto claims = requireClaims(req);
        auto user = registry.userRepository->findById(std::stoll(id));
        if (!user.has_value())
        {
            throw domain::DomainError("User not found");
        }
        if (user->role == domain::UserRole::SUPER_ADMIN)
        {
            throw domain::DomainError("SUPER_ADMIN users cannot be deactivated from this endpoint");
        }
        if (claims.businessId.has_value() && user->businessId != claims.businessId)
        {
            throw domain::DomainError("User does not belong to your business");
        }
        registry.userRepository->deactivate(user->id);
        user->isActive = false;
        return jsonResponse(successResponse(userToJson(*user)));
    });
}

void ApiController::listTables(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    executeSafely(callback, [&]() {
        Json::Value data(Json::arrayValue);
        for (const auto &table : registry.listTables->execute(resolveBusinessId(req)))
        {
            data.append(tableToJson(table));
        }
        return jsonResponse(successResponse(data));
    });
}

void ApiController::createTable(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    executeSafely(callback, [&]() {
        const auto &json = body(req);
        application::CreateTableCommand command;
        command.businessId = resolveBusinessId(req, &json);
        command.tableNumber = json["tableNumber"].asInt();
        return jsonResponse(successResponse(tableToJson(registry.createTable->execute(command))), drogon::k201Created);
    });
}

void ApiController::regenerateTableQr(const drogon::HttpRequestPtr &req,
                                      std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                                      std::string id)
{
    executeSafely(callback, [&]() {
        return jsonResponse(successResponse(tableToJson(registry.generateQrToken->execute(resolveBusinessId(req), std::stoll(id)))));
    });
}

void ApiController::deactivateTable(const drogon::HttpRequestPtr &req,
                                    std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                                    std::string id)
{
    executeSafely(callback, [&]() {
        registry.deactivateTable->execute(resolveBusinessId(req), std::stoll(id));
        return jsonResponse(successResponse(Json::Value("Table deactivated")));
    });
}

void ApiController::listCategories(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    executeSafely(callback, [&]() {
        const bool includeInactive = isTruthy(req->getParameter("includeInactive"));
        Json::Value data(Json::arrayValue);
        for (const auto &category : registry.listCategories->execute(resolveBusinessId(req), !includeInactive))
        {
            data.append(categoryToJson(category));
        }
        return jsonResponse(successResponse(data));
    });
}

void ApiController::createCategory(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    executeSafely(callback, [&]() {
        const auto &json = body(req);
        application::CreateCategoryCommand command;
        command.name = sanitizeText(json["name"].asString());
        command.description = sanitizeText(json.get("description", "").asString());
        return jsonResponse(successResponse(categoryToJson(registry.createCategory->execute(resolveBusinessId(req, &json), command))),
                            drogon::k201Created);
    });
}

void ApiController::listProducts(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    executeSafely(callback, [&]() {
        const bool includeInactive = isTruthy(req->getParameter("includeInactive"));
        Json::Value data(Json::arrayValue);
        for (const auto &product : registry.listProducts->execute(resolveBusinessId(req), !includeInactive))
        {
            data.append(productToJson(product));
        }
        return jsonResponse(successResponse(data));
    });
}

void ApiController::createProduct(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    executeSafely(callback, [&]() {
        const auto &json = body(req);
        application::CreateProductCommand command;
        command.businessId = resolveBusinessId(req, &json);
        command.categoryId = json["categoryId"].asInt64();
        command.name = sanitizeText(json["name"].asString());
        command.description = sanitizeText(json.get("description", "").asString());
        command.price = json["price"].asDouble();
        return jsonResponse(successResponse(productToJson(registry.createProduct->execute(command))), drogon::k201Created);
    });
}

void ApiController::updateProduct(const drogon::HttpRequestPtr &req,
                                  std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                                  std::string id)
{
    executeSafely(callback, [&]() {
        const auto &json = body(req);
        application::UpdateProductCommand command;
        command.businessId = resolveBusinessId(req, &json);
        command.categoryId = json["categoryId"].asInt64();
        command.name = sanitizeText(json["name"].asString());
        command.description = sanitizeText(json.get("description", "").asString());
        command.price = json["price"].asDouble();
        command.isAvailable = json.get("isAvailable", true).asBool();
        return jsonResponse(successResponse(productToJson(registry.updateProduct->execute(std::stoll(id), command))));
    });
}

void ApiController::activateProduct(const drogon::HttpRequestPtr &req,
                                    std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                                    std::string id)
{
    executeSafely(callback, [&]() {
        registry.activateProduct->execute(resolveBusinessId(req), std::stoll(id));
        return jsonResponse(successResponse(Json::Value("Product activated")));
    });
}

void ApiController::markProductUnavailable(const drogon::HttpRequestPtr &req,
                                           std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                                           std::string id)
{
    executeSafely(callback, [&]() {
        registry.markProductUnavailable->execute(resolveBusinessId(req), std::stoll(id));
        return jsonResponse(successResponse(Json::Value("Product marked as unavailable")));
    });
}

void ApiController::deactivateProduct(const drogon::HttpRequestPtr &req,
                                      std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                                      std::string id)
{
    executeSafely(callback, [&]() {
        registry.deactivateProduct->execute(resolveBusinessId(req), std::stoll(id));
        return jsonResponse(successResponse(Json::Value("Product deactivated")));
    });
}

void ApiController::listAddons(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    executeSafely(callback, [&]() {
        const bool includeInactive = isTruthy(req->getParameter("includeInactive"));
        Json::Value data(Json::arrayValue);
        for (const auto &addon : registry.addonRepository->listAll(resolveBusinessId(req), !includeInactive))
        {
            data.append(addonToJson(addon));
        }
        return jsonResponse(successResponse(data));
    });
}

void ApiController::createAddon(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    executeSafely(callback, [&]() {
        const auto &json = body(req);
        application::CreateAddonCommand command;
        command.businessId = resolveBusinessId(req, &json);
        command.name = sanitizeText(json["name"].asString());
        command.price = json["price"].asDouble();
        return jsonResponse(successResponse(addonToJson(registry.createAddon->execute(command))), drogon::k201Created);
    });
}

void ApiController::assignAddonToProduct(const drogon::HttpRequestPtr &req,
                                         std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                                         std::string productId,
                                         std::string addonId)
{
    executeSafely(callback, [&]() {
        registry.assignAddonToProduct->execute(resolveBusinessId(req), std::stoll(productId), std::stoll(addonId));
        return jsonResponse(successResponse(Json::Value("Addon assigned to product")), drogon::k201Created);
    });
}

void ApiController::listAdminOrders(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    executeSafely(callback, [&]() {
        Json::Value data(Json::arrayValue);
        for (const auto &order : registry.getKitchenOrders->execute(resolveBusinessId(req)))
        {
            data.append(orderToJson(order));
        }
        return jsonResponse(successResponse(data));
    });
}

void ApiController::listAdminOrdersHistory(const drogon::HttpRequestPtr &req,
                                           std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    executeSafely(callback, [&]() {
        Json::Value data(Json::arrayValue);
        for (const auto &order : registry.getOrdersHistory->execute(resolveBusinessId(req)))
        {
            data.append(orderToJson(order));
        }
        return jsonResponse(successResponse(data));
    });
}

void ApiController::cancelOrder(const drogon::HttpRequestPtr &req,
                                std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                                std::string orderId)
{
    executeSafely(callback, [&]() {
        registry.cancelOrder->execute(resolveBusinessId(req), std::stoll(orderId));
        return jsonResponse(successResponse(Json::Value("Order cancelled")));
    });
}

void ApiController::searchCashierOrders(const drogon::HttpRequestPtr &req,
                                        std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    executeSafely(callback, [&]() {
        Json::Value data(Json::arrayValue);
        for (const auto &order : registry.searchOrdersForCashier->execute(resolveBusinessId(req),
                                                                           sanitizeText(req->getParameter("customerName")),
                                                                           sanitizeText(req->getParameter("tableNumber")),
                                                                           sanitizeText(req->getParameter("status"))))
        {
            data.append(orderToJson(order));
        }
        return jsonResponse(successResponse(data));
    });
}

void ApiController::payOrder(const drogon::HttpRequestPtr &req,
                             std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                             std::string orderId)
{
    executeSafely(callback, [&]() {
        const auto &json = body(req);
        application::PayOrderCommand command;
        command.businessId = resolveBusinessId(req, &json);
        command.orderId = std::stoll(orderId);
        command.amount = json["amount"].asDouble();
        return jsonResponse(successResponse(paymentToJson(registry.payOrder->execute(command))), drogon::k201Created);
    });
}
}  // namespace starcafe::interfaces::rest




