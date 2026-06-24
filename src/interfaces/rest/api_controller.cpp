#include "interfaces/rest/api_controller.h"

#include "application/common/dtos.h"
#include "domain/common/enums.h"
#include "domain/common/errors.h"
#include "infrastructure/security/auth_filters.h"
#include "interfaces/rest/service_registry.h"

#include <json/json.h>

namespace starcafe::interfaces::rest
{
namespace
{
ServiceRegistry registry;

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

Json::Value userToJson(const domain::identity::User &user)
{
    Json::Value value;
    value["id"] = Json::Int64(user.id);
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
    value["tableNumber"] = table.tableNumber;
    value["qrToken"] = table.qrToken;
    value["isActive"] = table.isActive;
    return value;
}

Json::Value addonToJson(const domain::menu::Addon &addon)
{
    Json::Value value;
    value["id"] = Json::Int64(addon.id);
    value["name"] = addon.name;
    value["price"] = addon.price;
    value["isActive"] = addon.isActive;
    return value;
}

Json::Value productToJson(const domain::menu::Product &product)
{
    Json::Value value;
    value["id"] = Json::Int64(product.id);
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
        image["url"] = services().publicProductFilesBaseUrl + "/" + product.image->fileName;
        value["image"] = image;
    }
    else
    {
        value["image"] = Json::nullValue;
    }
    return value;
}

Json::Value categoryToJson(const domain::menu::Category &category)
{
    Json::Value value;
    value["id"] = Json::Int64(category.id);
    value["name"] = category.name;
    value["description"] = category.description;
    value["isActive"] = category.isActive;
    return value;
}

Json::Value orderToJson(const domain::orders::Order &order)
{
    Json::Value value;
    value["id"] = Json::Int64(order.id);
    value["tableId"] = Json::Int64(order.tableId);
    value["tableNumber"] = order.tableNumber;
    value["customerName"] = order.customerName;
    value["status"] = domain::toString(order.status);
    value["total"] = order.total;
    value["createdAt"] = order.createdAt;
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
    value["orderId"] = Json::Int64(payment.orderId);
    value["amount"] = payment.amount;
    value["status"] = domain::toString(payment.status);
    value["paidAt"] = payment.paidAt;
    return value;
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
}  // namespace

ServiceRegistry &services() { return registry; }

void ApiController::registerUser(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    executeSafely(callback, [&]() {
        const auto &json = body(req);
        application::RegisterUserCommand command{
            json["name"].asString(),
            json["email"].asString(),
            json["password"].asString(),
            domain::userRoleFromString(json["role"].asString())};
        return jsonResponse(successResponse(userToJson(registry.registerUser->execute(command))), drogon::k201Created);
    });
}

void ApiController::login(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    executeSafely(callback, [&]() {
        const auto &json = body(req);
        const auto auth = registry.login->execute({json["email"].asString(), json["password"].asString()});
        Json::Value data;
        data["token"] = auth.token;
        data["userId"] = Json::Int64(auth.userId);
        data["role"] = auth.role;
        data["name"] = auth.name;
        data["email"] = auth.email;
        return jsonResponse(successResponse(data));
    });
}

void ApiController::me(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    executeSafely(callback, [&]() {
        const auto claims = infrastructure::security::parseBearerClaims(req);
        if (!claims.has_value())
        {
            throw domain::DomainError("Invalid token");
        }
        return jsonResponse(successResponse(userToJson(registry.getCurrentUser->execute(claims->userId))));
    });
}

void ApiController::getTableByQr(const drogon::HttpRequestPtr &, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string qrToken)
{
    executeSafely(callback, [&]() { return jsonResponse(successResponse(tableToJson(registry.getTableByQrToken->execute(qrToken)))); });
}

void ApiController::getPublicMenu(const drogon::HttpRequestPtr &, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    executeSafely(callback, [&]() {
        Json::Value data(Json::arrayValue);
        for (const auto &product : registry.getPublicMenu->execute())
            data.append(productToJson(product));
        return jsonResponse(successResponse(data));
    });
}

void ApiController::createOrderFromTable(const drogon::HttpRequestPtr &req,
                                         std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                                         std::string qrToken)
{
    executeSafely(callback, [&]() {
        const auto &json = body(req);
        application::CreateOrderFromTableCommand command;
        command.qrToken = qrToken;
        command.customerName = json["customerName"].asString();
        for (const auto &item : json["items"])
        {
            application::CreateOrderItemCommand itemCommand;
            itemCommand.productId = item["productId"].asInt64();
            itemCommand.quantity = item["quantity"].asInt();
            itemCommand.notes = item["notes"].asString();
            for (const auto &addonId : item["addonIds"])
                itemCommand.addonIds.push_back(addonId.asInt64());
            command.items.push_back(itemCommand);
        }
        return jsonResponse(successResponse(orderToJson(registry.createOrderFromTable->execute(command))), drogon::k201Created);
    });
}

void ApiController::getOrderStatus(const drogon::HttpRequestPtr &, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string orderId)
{
    executeSafely(callback, [&]() { return jsonResponse(successResponse(orderToJson(registry.getOrderStatus->execute(std::stoll(orderId))))); });
}

void ApiController::getKitchenOrders(const drogon::HttpRequestPtr &, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    executeSafely(callback, [&]() {
        Json::Value data(Json::arrayValue);
        for (const auto &order : registry.getKitchenOrders->execute())
            data.append(orderToJson(order));
        return jsonResponse(successResponse(data));
    });
}

void ApiController::startPreparingOrder(const drogon::HttpRequestPtr &, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string orderId)
{
    executeSafely(callback, [&]() {
        registry.startPreparingOrder->execute(std::stoll(orderId));
        Json::Value data;
        data["message"] = "Order moved to PREPARING";
        return jsonResponse(successResponse(data));
    });
}

void ApiController::markOrderItemReady(const drogon::HttpRequestPtr &, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string itemId)
{
    executeSafely(callback, [&]() {
        registry.markOrderItemReady->execute(std::stoll(itemId));
        Json::Value data;
        data["message"] = "Order item moved to READY";
        return jsonResponse(successResponse(data));
    });
}

void ApiController::markOrderReady(const drogon::HttpRequestPtr &, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string orderId)
{
    executeSafely(callback, [&]() {
        registry.markOrderReady->execute(std::stoll(orderId));
        Json::Value data;
        data["message"] = "Order moved to READY";
        return jsonResponse(successResponse(data));
    });
}

void ApiController::getOrdersHistory(const drogon::HttpRequestPtr &, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    executeSafely(callback, [&]() {
        Json::Value data(Json::arrayValue);
        for (const auto &order : registry.getOrdersHistory->execute())
            data.append(orderToJson(order));
        return jsonResponse(successResponse(data));
    });
}

void ApiController::listUsers(const drogon::HttpRequestPtr &, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    executeSafely(callback, [&]() {
        Json::Value data(Json::arrayValue);
        for (const auto &user : registry.userRepository->listActive())
        {
            data.append(userToJson(user));
        }
        return jsonResponse(successResponse(data));
    });
}

void ApiController::createAdminUser(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    registerUser(req, std::move(callback));
}

void ApiController::deactivateUser(const drogon::HttpRequestPtr &, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string id)
{
    executeSafely(callback, [&]() {
        registry.userRepository->deactivate(std::stoll(id));
        return jsonResponse(successResponse(Json::Value("User deactivated")));
    });
}

void ApiController::listTables(const drogon::HttpRequestPtr &, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    executeSafely(callback, [&]() {
        Json::Value data(Json::arrayValue);
        for (const auto &table : registry.listTables->execute())
            data.append(tableToJson(table));
        return jsonResponse(successResponse(data));
    });
}

void ApiController::createTable(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    executeSafely(callback, [&]() {
        const auto &json = body(req);
        return jsonResponse(successResponse(tableToJson(registry.createTable->execute({json["tableNumber"].asInt()}))), drogon::k201Created);
    });
}

void ApiController::deactivateTable(const drogon::HttpRequestPtr &, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string id)
{
    executeSafely(callback, [&]() {
        registry.deactivateTable->execute(std::stoll(id));
        return jsonResponse(successResponse(Json::Value("Table deactivated")));
    });
}

void ApiController::listCategories(const drogon::HttpRequestPtr &, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    executeSafely(callback, [&]() {
        Json::Value data(Json::arrayValue);
        for (const auto &category : registry.listCategories->execute(false))
            data.append(categoryToJson(category));
        return jsonResponse(successResponse(data));
    });
}

void ApiController::createCategory(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    executeSafely(callback, [&]() {
        const auto &json = body(req);
        return jsonResponse(successResponse(categoryToJson(registry.createCategory->execute({json["name"].asString(), json["description"].asString()}))),
                            drogon::k201Created);
    });
}

void ApiController::listProducts(const drogon::HttpRequestPtr &, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    executeSafely(callback, [&]() {
        Json::Value data(Json::arrayValue);
        for (const auto &product : registry.listProducts->execute(false))
            data.append(productToJson(product));
        return jsonResponse(successResponse(data));
    });
}

void ApiController::createProduct(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    executeSafely(callback, [&]() {
        const auto &json = body(req);
        return jsonResponse(successResponse(productToJson(registry.createProduct->execute(
                                {json["categoryId"].asInt64(), json["name"].asString(), json["description"].asString(), json["price"].asDouble()}))),
                            drogon::k201Created);
    });
}

void ApiController::updateProduct(const drogon::HttpRequestPtr &req,
                                  std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                                  std::string id)
{
    executeSafely(callback, [&]() {
        const auto &json = body(req);
        application::UpdateProductCommand command{
            json["categoryId"].asInt64(),
            json["name"].asString(),
            json["description"].asString(),
            json["price"].asDouble(),
            json.get("isAvailable", true).asBool()};
        return jsonResponse(successResponse(productToJson(registry.updateProduct->execute(std::stoll(id), command))));
    });
}

void ApiController::markProductUnavailable(const drogon::HttpRequestPtr &, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string id)
{
    executeSafely(callback, [&]() {
        registry.markProductUnavailable->execute(std::stoll(id));
        return jsonResponse(successResponse(Json::Value("Product marked as unavailable")));
    });
}

void ApiController::deactivateProduct(const drogon::HttpRequestPtr &, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string id)
{
    executeSafely(callback, [&]() {
        registry.deactivateProduct->execute(std::stoll(id));
        return jsonResponse(successResponse(Json::Value("Product deactivated")));
    });
}

void ApiController::listAddons(const drogon::HttpRequestPtr &, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    executeSafely(callback, [&]() {
        Json::Value data(Json::arrayValue);
        for (const auto &addon : registry.addonRepository->listAll(false))
            data.append(addonToJson(addon));
        return jsonResponse(successResponse(data));
    });
}

void ApiController::createAddon(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    executeSafely(callback, [&]() {
        const auto &json = body(req);
        return jsonResponse(successResponse(addonToJson(registry.createAddon->execute({json["name"].asString(), json["price"].asDouble()}))),
                            drogon::k201Created);
    });
}

void ApiController::assignAddonToProduct(const drogon::HttpRequestPtr &,
                                         std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                                         std::string productId,
                                         std::string addonId)
{
    executeSafely(callback, [&]() {
        registry.assignAddonToProduct->execute(std::stoll(productId), std::stoll(addonId));
        return jsonResponse(successResponse(Json::Value("Addon assigned")));
    });
}

void ApiController::listAdminOrders(const drogon::HttpRequestPtr &, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    executeSafely(callback, [&]() {
        Json::Value data(Json::arrayValue);
        for (const auto &order : registry.getKitchenOrders->execute())
            data.append(orderToJson(order));
        return jsonResponse(successResponse(data));
    });
}

void ApiController::listAdminOrdersHistory(const drogon::HttpRequestPtr &, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    executeSafely(callback, [&]() {
        Json::Value data(Json::arrayValue);
        for (const auto &order : registry.getOrdersHistory->execute())
            data.append(orderToJson(order));
        return jsonResponse(successResponse(data));
    });
}

void ApiController::cancelOrder(const drogon::HttpRequestPtr &, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string orderId)
{
    executeSafely(callback, [&]() {
        registry.cancelOrder->execute(std::stoll(orderId));
        return jsonResponse(successResponse(Json::Value("Order cancelled")));
    });
}

void ApiController::searchCashierOrders(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    executeSafely(callback, [&]() {
        Json::Value data(Json::arrayValue);
        for (const auto &order : registry.searchOrdersForCashier->execute(req->getParameter("customerName"),
                                                                          req->getParameter("tableNumber"),
                                                                          req->getParameter("status")))
            data.append(orderToJson(order));
        return jsonResponse(successResponse(data));
    });
}

void ApiController::payOrder(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string orderId)
{
    executeSafely(callback, [&]() {
        const auto &json = body(req);
        application::PayOrderCommand command{std::stoll(orderId), json["amount"].asDouble()};
        return jsonResponse(successResponse(paymentToJson(registry.payOrder->execute(command))));
    });
}
}  // namespace starcafe::interfaces::rest
