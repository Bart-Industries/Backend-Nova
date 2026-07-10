#include "interfaces/rest/docs_controller.h"

#include <drogon/HttpResponse.h>

namespace starcafe::interfaces::rest
{
namespace
{
Json::Value makeJsonBodySchema(const std::initializer_list<std::pair<std::string, std::string>> &fields)
{
    Json::Value schema;
    schema["type"] = "object";
    for (const auto &[name, type] : fields)
    {
        schema["properties"][name]["type"] = type;
    }
    return schema;
}

void addJsonRequestBody(Json::Value &operation,
                        const std::initializer_list<std::pair<std::string, std::string>> &fields,
                        const std::initializer_list<std::string> &required = {})
{
    auto schema = makeJsonBodySchema(fields);
    for (const auto &name : required)
    {
        schema["required"].append(name);
    }
    operation["requestBody"]["required"] = true;
    operation["requestBody"]["content"]["application/json"]["schema"] = schema;
}

void addMultipartRequestBody(Json::Value &operation, const std::string &fieldName = "image")
{
    Json::Value schema;
    schema["type"] = "object";
    schema["required"].append(fieldName);
    schema["properties"][fieldName]["type"] = "string";
    schema["properties"][fieldName]["format"] = "binary";
    operation["requestBody"]["required"] = true;
    operation["requestBody"]["content"]["multipart/form-data"]["schema"] = schema;
}

void addCreateOrderRequestBody(Json::Value &operation)
{
    Json::Value itemSchema;
    itemSchema["type"] = "object";
    itemSchema["required"].append("productId");
    itemSchema["required"].append("quantity");
    itemSchema["properties"]["productId"]["type"] = "integer";
    itemSchema["properties"]["quantity"]["type"] = "integer";
    itemSchema["properties"]["notes"]["type"] = "string";
    itemSchema["properties"]["addonIds"]["type"] = "array";
    itemSchema["properties"]["addonIds"]["items"]["type"] = "integer";

    Json::Value schema;
    schema["type"] = "object";
    schema["required"].append("customerName");
    schema["required"].append("items");
    schema["properties"]["customerName"]["type"] = "string";
    schema["properties"]["items"]["type"] = "array";
    schema["properties"]["items"]["items"] = itemSchema;

    operation["requestBody"]["required"] = true;
    operation["requestBody"]["content"]["application/json"]["schema"] = schema;
}

Json::Value successResponseSchema()
{
    Json::Value schema;
    schema["type"] = "object";
    schema["properties"]["success"]["type"] = "boolean";
    schema["properties"]["data"]["type"] = "object";
    return schema;
}

Json::Value errorResponseSchema()
{
    Json::Value schema;
    schema["type"] = "object";
    schema["properties"]["success"]["type"] = "boolean";
    schema["properties"]["message"]["type"] = "string";
    return schema;
}

void attachDefaultResponses(Json::Value &operation)
{
    operation["responses"]["200"]["description"] = "OK";
    operation["responses"]["200"]["content"]["application/json"]["schema"] = successResponseSchema();
    operation["responses"]["201"]["description"] = "Created";
    operation["responses"]["201"]["content"]["application/json"]["schema"] = successResponseSchema();
    operation["responses"]["400"]["description"] = "Bad Request";
    operation["responses"]["400"]["content"]["application/json"]["schema"] = errorResponseSchema();
    operation["responses"]["401"]["description"] = "Unauthorized";
    operation["responses"]["401"]["content"]["application/json"]["schema"] = errorResponseSchema();
}

Json::Value bearerSecurity()
{
    Json::Value security(Json::arrayValue);
    Json::Value item;
    item["bearerAuth"] = Json::arrayValue;
    security.append(item);
    return security;
}

void addPathParameter(Json::Value &operation, const std::string &name, const std::string &description)
{
    Json::Value parameter;
    parameter["in"] = "path";
    parameter["name"] = name;
    parameter["required"] = true;
    parameter["description"] = description;
    parameter["schema"]["type"] = "string";
    operation["parameters"].append(parameter);
}

void addQueryParameter(Json::Value &operation, const std::string &name, const std::string &description)
{
    Json::Value parameter;
    parameter["in"] = "query";
    parameter["name"] = name;
    parameter["required"] = false;
    parameter["description"] = description;
    parameter["schema"]["type"] = "string";
    operation["parameters"].append(parameter);
}

void addTag(Json::Value &operation, const std::string &tag)
{
    operation["tags"].append(tag);
}
}  // namespace

void DocsController::swaggerUi(const drogon::HttpRequestPtr &, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    static const std::string html = R"HTML(
<!doctype html>
<html lang="en">
  <head>
    <meta charset="utf-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1" />
    <title>Nova API Docs</title>
    <link rel="stylesheet" href="https://unpkg.com/swagger-ui-dist@5/swagger-ui.css" />
    <style>
      body { margin: 0; background: #faf7f0; }
      .topbar { display: none; }
    </style>
  </head>
  <body>
    <div id="swagger-ui"></div>
    <script src="https://unpkg.com/swagger-ui-dist@5/swagger-ui-bundle.js"></script>
    <script>
      window.ui = SwaggerUIBundle({
        url: '/openapi.json',
        dom_id: '#swagger-ui',
        deepLinking: true,
        displayRequestDuration: true,
        persistAuthorization: true,
        tagsSorter: 'alpha',
        operationsSorter: 'alpha',
        docExpansion: 'list'
      });
    </script>
  </body>
</html>
)HTML";

    auto response = drogon::HttpResponse::newHttpResponse();
    response->setContentTypeCode(drogon::CT_TEXT_HTML);
    response->setBody(html);
    callback(response);
}

void DocsController::openApiJson(const drogon::HttpRequestPtr &, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    Json::Value root;
    root["openapi"] = "3.0.3";
    root["info"]["title"] = "Nova API";
    root["info"]["version"] = "2.0.0";
    root["info"]["description"] = "Documentacion para pruebas del backend multi-cafeteria de Nova.";
    root["tags"][0]["name"] = "Super Admin Businesses";
    root["tags"][0]["description"] = "Gestion global de cafeterias dentro de la plataforma Nova.";
    root["tags"][1]["name"] = "Auth";
    root["tags"][1]["description"] = "Autenticacion y perfil del usuario autenticado.";
    root["tags"][2]["name"] = "Public";
    root["tags"][2]["description"] = "Endpoints publicos para clientes que ingresan por QR.";
    root["tags"][3]["name"] = "Kitchen";
    root["tags"][3]["description"] = "Flujo operativo de cocina.";
    root["tags"][4]["name"] = "Admin Users";
    root["tags"][4]["description"] = "Gestion de usuarios administrativos y de cocina por cafeteria.";
    root["tags"][5]["name"] = "Admin Tables";
    root["tags"][5]["description"] = "Gestion de mesas y QR por cafeteria.";
    root["tags"][6]["name"] = "Admin Categories";
    root["tags"][6]["description"] = "Gestion de categorias del menu por cafeteria.";
    root["tags"][7]["name"] = "Admin Products";
    root["tags"][7]["description"] = "Gestion de productos del menu por cafeteria.";
    root["tags"][8]["name"] = "Admin Product Images";
    root["tags"][8]["description"] = "Carga, reemplazo y eliminacion de imagenes de producto.";
    root["tags"][9]["name"] = "Admin Addons";
    root["tags"][9]["description"] = "Gestion de adicionales y asignaciones a productos.";
    root["tags"][10]["name"] = "Admin Orders";
    root["tags"][10]["description"] = "Supervision y acciones administrativas sobre pedidos.";
    root["tags"][11]["name"] = "Cashier";
    root["tags"][11]["description"] = "Busqueda y pago de pedidos en caja.";
    root["tags"][12]["name"] = "Admin Business Branding";
    root["tags"][12]["description"] = "Configuracion de branding por cafeteria para usuarios ADMIN.";

    root["components"]["securitySchemes"]["bearerAuth"]["type"] = "http";
    root["components"]["securitySchemes"]["bearerAuth"]["scheme"] = "bearer";
    root["components"]["securitySchemes"]["bearerAuth"]["bearerFormat"] = "JWT";

    auto &paths = root["paths"];

    paths["/api/v1/super-admin/businesses"]["get"]["summary"] = "Listar cafeterias";
    addTag(paths["/api/v1/super-admin/businesses"]["get"], "Super Admin Businesses");
    paths["/api/v1/super-admin/businesses"]["get"]["security"] = bearerSecurity();
    addQueryParameter(paths["/api/v1/super-admin/businesses"]["get"], "includeInactive", "Incluir cafeterias inactivas");
    attachDefaultResponses(paths["/api/v1/super-admin/businesses"]["get"]);

    paths["/api/v1/super-admin/businesses"]["post"]["summary"] = "Crear cafeteria";
    addTag(paths["/api/v1/super-admin/businesses"]["post"], "Super Admin Businesses");
    paths["/api/v1/super-admin/businesses"]["post"]["security"] = bearerSecurity();
    addJsonRequestBody(paths["/api/v1/super-admin/businesses"]["post"],
                       {{"name", "string"}, {"slug", "string"}, {"logoUrl", "string"}, {"primaryColor", "string"}},
                       {"name", "slug"});
    attachDefaultResponses(paths["/api/v1/super-admin/businesses"]["post"]);

    paths["/api/v1/auth/register"]["post"]["summary"] = "Registrar usuario";
    addTag(paths["/api/v1/auth/register"]["post"], "Auth");
    addJsonRequestBody(paths["/api/v1/auth/register"]["post"],
                       {{"name", "string"}, {"email", "string"}, {"password", "string"}, {"role", "string"}, {"businessId", "integer"}},
                       {"name", "email", "password", "role"});
    attachDefaultResponses(paths["/api/v1/auth/register"]["post"]);

    paths["/api/v1/auth/login"]["post"]["summary"] = "Login";
    addTag(paths["/api/v1/auth/login"]["post"], "Auth");
    addJsonRequestBody(paths["/api/v1/auth/login"]["post"], {{"email", "string"}, {"password", "string"}}, {"email", "password"});
    attachDefaultResponses(paths["/api/v1/auth/login"]["post"]);

    paths["/api/v1/auth/me"]["get"]["summary"] = "Usuario autenticado";
    addTag(paths["/api/v1/auth/me"]["get"], "Auth");
    paths["/api/v1/auth/me"]["get"]["security"] = bearerSecurity();
    attachDefaultResponses(paths["/api/v1/auth/me"]["get"]);

    paths["/api/v1/tables/qr/{qrToken}"]["get"]["summary"] = "Obtener mesa por QR";
    addTag(paths["/api/v1/tables/qr/{qrToken}"]["get"], "Public");
    addPathParameter(paths["/api/v1/tables/qr/{qrToken}"]["get"], "qrToken", "Token del QR");
    attachDefaultResponses(paths["/api/v1/tables/qr/{qrToken}"]["get"]);

    paths["/api/v1/public/tables/{qrToken}/session"]["get"]["summary"] = "Obtener mesa y pedidos activos por QR";
    addTag(paths["/api/v1/public/tables/{qrToken}/session"]["get"], "Public");
    addPathParameter(paths["/api/v1/public/tables/{qrToken}/session"]["get"], "qrToken", "Token del QR");
    attachDefaultResponses(paths["/api/v1/public/tables/{qrToken}/session"]["get"]);

    paths["/api/v1/public/menu"]["get"]["summary"] = "Listar menu publico";
    addTag(paths["/api/v1/public/menu"]["get"], "Public");
    addQueryParameter(paths["/api/v1/public/menu"]["get"], "businessSlug", "Slug de la cafeteria");
    attachDefaultResponses(paths["/api/v1/public/menu"]["get"]);

    paths["/api/v1/public/tables/{qrToken}/orders"]["post"]["summary"] = "Crear pedido desde mesa";
    addTag(paths["/api/v1/public/tables/{qrToken}/orders"]["post"], "Public");
    addPathParameter(paths["/api/v1/public/tables/{qrToken}/orders"]["post"], "qrToken", "Token del QR");
    addCreateOrderRequestBody(paths["/api/v1/public/tables/{qrToken}/orders"]["post"]);
    attachDefaultResponses(paths["/api/v1/public/tables/{qrToken}/orders"]["post"]);

    paths["/api/v1/public/orders/{orderId}/status"]["get"]["summary"] = "Estado de pedido";
    addTag(paths["/api/v1/public/orders/{orderId}/status"]["get"], "Public");
    addPathParameter(paths["/api/v1/public/orders/{orderId}/status"]["get"], "orderId", "Id del pedido");
    addQueryParameter(paths["/api/v1/public/orders/{orderId}/status"]["get"], "qrToken", "Token QR de la mesa dueña del pedido");
    attachDefaultResponses(paths["/api/v1/public/orders/{orderId}/status"]["get"]);

    paths["/api/v1/kitchen/orders"]["get"]["summary"] = "Pedidos activos de cocina";
    addTag(paths["/api/v1/kitchen/orders"]["get"], "Kitchen");
    paths["/api/v1/kitchen/orders"]["get"]["security"] = bearerSecurity();
    attachDefaultResponses(paths["/api/v1/kitchen/orders"]["get"]);

    paths["/api/v1/kitchen/orders/{orderId}/preparing"]["patch"]["summary"] = "Mover pedido a PREPARING";
    addTag(paths["/api/v1/kitchen/orders/{orderId}/preparing"]["patch"], "Kitchen");
    paths["/api/v1/kitchen/orders/{orderId}/preparing"]["patch"]["security"] = bearerSecurity();
    addPathParameter(paths["/api/v1/kitchen/orders/{orderId}/preparing"]["patch"], "orderId", "Id del pedido");
    attachDefaultResponses(paths["/api/v1/kitchen/orders/{orderId}/preparing"]["patch"]);

    paths["/api/v1/kitchen/order-items/{itemId}/ready"]["patch"]["summary"] = "Marcar item como READY";
    addTag(paths["/api/v1/kitchen/order-items/{itemId}/ready"]["patch"], "Kitchen");
    paths["/api/v1/kitchen/order-items/{itemId}/ready"]["patch"]["security"] = bearerSecurity();
    addPathParameter(paths["/api/v1/kitchen/order-items/{itemId}/ready"]["patch"], "itemId", "Id del item");
    attachDefaultResponses(paths["/api/v1/kitchen/order-items/{itemId}/ready"]["patch"]);

    paths["/api/v1/kitchen/orders/{orderId}/ready"]["patch"]["summary"] = "Marcar pedido como READY";
    addTag(paths["/api/v1/kitchen/orders/{orderId}/ready"]["patch"], "Kitchen");
    paths["/api/v1/kitchen/orders/{orderId}/ready"]["patch"]["security"] = bearerSecurity();
    addPathParameter(paths["/api/v1/kitchen/orders/{orderId}/ready"]["patch"], "orderId", "Id del pedido");
    attachDefaultResponses(paths["/api/v1/kitchen/orders/{orderId}/ready"]["patch"]);

    paths["/api/v1/kitchen/orders/history"]["get"]["summary"] = "Historial de cocina";
    addTag(paths["/api/v1/kitchen/orders/history"]["get"], "Kitchen");
    paths["/api/v1/kitchen/orders/history"]["get"]["security"] = bearerSecurity();
    attachDefaultResponses(paths["/api/v1/kitchen/orders/history"]["get"]);

    paths["/api/v1/admin/users"]["get"]["summary"] = "Listar usuarios";
    addTag(paths["/api/v1/admin/users"]["get"], "Admin Users");
    paths["/api/v1/admin/users"]["get"]["security"] = bearerSecurity();
    addQueryParameter(paths["/api/v1/admin/users"]["get"], "businessId", "Contexto de cafeteria para SUPER_ADMIN");
    attachDefaultResponses(paths["/api/v1/admin/users"]["get"]);

    paths["/api/v1/admin/users"]["post"]["summary"] = "Crear usuario";
    addTag(paths["/api/v1/admin/users"]["post"], "Admin Users");
    paths["/api/v1/admin/users"]["post"]["security"] = bearerSecurity();
    addJsonRequestBody(paths["/api/v1/admin/users"]["post"],
                       {{"name", "string"}, {"email", "string"}, {"password", "string"}, {"role", "string"}, {"businessId", "integer"}},
                       {"name", "email", "password", "role"});
    attachDefaultResponses(paths["/api/v1/admin/users"]["post"]);

    paths["/api/v1/admin/users/{id}/deactivate"]["patch"]["summary"] = "Desactivar usuario";
    addTag(paths["/api/v1/admin/users/{id}/deactivate"]["patch"], "Admin Users");
    paths["/api/v1/admin/users/{id}/deactivate"]["patch"]["security"] = bearerSecurity();
    addPathParameter(paths["/api/v1/admin/users/{id}/deactivate"]["patch"], "id", "Id del usuario");
    attachDefaultResponses(paths["/api/v1/admin/users/{id}/deactivate"]["patch"]);

    paths["/api/v1/admin/business"]["get"]["summary"] = "Obtener branding de la cafeteria actual";
    addTag(paths["/api/v1/admin/business"]["get"], "Admin Business Branding");
    paths["/api/v1/admin/business"]["get"]["security"] = bearerSecurity();
    attachDefaultResponses(paths["/api/v1/admin/business"]["get"]);

    paths["/api/v1/admin/business/theme"]["patch"]["summary"] = "Actualizar tema predefinido de la cafeteria";
    addTag(paths["/api/v1/admin/business/theme"]["patch"], "Admin Business Branding");
    paths["/api/v1/admin/business/theme"]["patch"]["security"] = bearerSecurity();
    addJsonRequestBody(paths["/api/v1/admin/business/theme"]["patch"], {{"themeKey", "string"}}, {"themeKey"});
    attachDefaultResponses(paths["/api/v1/admin/business/theme"]["patch"]);

    paths["/api/v1/admin/business/logo"]["patch"]["summary"] = "Actualizar logo de la cafeteria";
    addTag(paths["/api/v1/admin/business/logo"]["patch"], "Admin Business Branding");
    paths["/api/v1/admin/business/logo"]["patch"]["security"] = bearerSecurity();
    addMultipartRequestBody(paths["/api/v1/admin/business/logo"]["patch"], "logo");
    attachDefaultResponses(paths["/api/v1/admin/business/logo"]["patch"]);

    paths["/api/v1/admin/tables"]["get"]["summary"] = "Listar mesas";
    addTag(paths["/api/v1/admin/tables"]["get"], "Admin Tables");
    paths["/api/v1/admin/tables"]["get"]["security"] = bearerSecurity();
    addQueryParameter(paths["/api/v1/admin/tables"]["get"], "businessId", "Contexto de cafeteria para SUPER_ADMIN");
    attachDefaultResponses(paths["/api/v1/admin/tables"]["get"]);

    paths["/api/v1/admin/tables"]["post"]["summary"] = "Crear mesa";
    addTag(paths["/api/v1/admin/tables"]["post"], "Admin Tables");
    paths["/api/v1/admin/tables"]["post"]["security"] = bearerSecurity();
    addJsonRequestBody(paths["/api/v1/admin/tables"]["post"], {{"tableNumber", "integer"}, {"businessId", "integer"}}, {"tableNumber"});
    attachDefaultResponses(paths["/api/v1/admin/tables"]["post"]);

    paths["/api/v1/admin/tables/{id}/regenerate-qr"]["patch"]["summary"] = "Regenerar QR de mesa";
    addTag(paths["/api/v1/admin/tables/{id}/regenerate-qr"]["patch"], "Admin Tables");
    paths["/api/v1/admin/tables/{id}/regenerate-qr"]["patch"]["security"] = bearerSecurity();
    addPathParameter(paths["/api/v1/admin/tables/{id}/regenerate-qr"]["patch"], "id", "Id de la mesa");
    attachDefaultResponses(paths["/api/v1/admin/tables/{id}/regenerate-qr"]["patch"]);

    paths["/api/v1/admin/tables/{id}/deactivate"]["patch"]["summary"] = "Desactivar mesa";
    addTag(paths["/api/v1/admin/tables/{id}/deactivate"]["patch"], "Admin Tables");
    paths["/api/v1/admin/tables/{id}/deactivate"]["patch"]["security"] = bearerSecurity();
    addPathParameter(paths["/api/v1/admin/tables/{id}/deactivate"]["patch"], "id", "Id de la mesa");
    attachDefaultResponses(paths["/api/v1/admin/tables/{id}/deactivate"]["patch"]);

    paths["/api/v1/admin/categories"]["get"]["summary"] = "Listar categorias";
    addTag(paths["/api/v1/admin/categories"]["get"], "Admin Categories");
    paths["/api/v1/admin/categories"]["get"]["security"] = bearerSecurity();
    addQueryParameter(paths["/api/v1/admin/categories"]["get"], "businessId", "Contexto de cafeteria para SUPER_ADMIN");
    addQueryParameter(paths["/api/v1/admin/categories"]["get"], "includeInactive", "Incluir categorias inactivas");
    attachDefaultResponses(paths["/api/v1/admin/categories"]["get"]);

    paths["/api/v1/admin/categories"]["post"]["summary"] = "Crear categoria";
    addTag(paths["/api/v1/admin/categories"]["post"], "Admin Categories");
    paths["/api/v1/admin/categories"]["post"]["security"] = bearerSecurity();
    addJsonRequestBody(paths["/api/v1/admin/categories"]["post"], {{"name", "string"}, {"description", "string"}, {"businessId", "integer"}}, {"name"});
    attachDefaultResponses(paths["/api/v1/admin/categories"]["post"]);

    paths["/api/v1/admin/products"]["get"]["summary"] = "Listar productos";
    addTag(paths["/api/v1/admin/products"]["get"], "Admin Products");
    paths["/api/v1/admin/products"]["get"]["security"] = bearerSecurity();
    addQueryParameter(paths["/api/v1/admin/products"]["get"], "businessId", "Contexto de cafeteria para SUPER_ADMIN");
    addQueryParameter(paths["/api/v1/admin/products"]["get"], "includeInactive", "Incluir productos inactivos");
    attachDefaultResponses(paths["/api/v1/admin/products"]["get"]);

    paths["/api/v1/admin/products"]["post"]["summary"] = "Crear producto";
    addTag(paths["/api/v1/admin/products"]["post"], "Admin Products");
    paths["/api/v1/admin/products"]["post"]["security"] = bearerSecurity();
    addJsonRequestBody(paths["/api/v1/admin/products"]["post"],
                       {{"businessId", "integer"}, {"categoryId", "integer"}, {"name", "string"}, {"description", "string"}, {"price", "number"}},
                       {"categoryId", "name", "price"});
    attachDefaultResponses(paths["/api/v1/admin/products"]["post"]);

    paths["/api/v1/admin/products/{id}"]["patch"]["summary"] = "Actualizar producto";
    addTag(paths["/api/v1/admin/products/{id}"]["patch"], "Admin Products");
    paths["/api/v1/admin/products/{id}"]["patch"]["security"] = bearerSecurity();
    addPathParameter(paths["/api/v1/admin/products/{id}"]["patch"], "id", "Id del producto");
    addJsonRequestBody(paths["/api/v1/admin/products/{id}"]["patch"],
                       {{"businessId", "integer"}, {"categoryId", "integer"}, {"name", "string"}, {"description", "string"}, {"price", "number"}, {"isAvailable", "boolean"}});
    attachDefaultResponses(paths["/api/v1/admin/products/{id}"]["patch"]);

    paths["/api/v1/admin/products/{id}/activate"]["patch"]["summary"] = "Reactivar producto";
    addTag(paths["/api/v1/admin/products/{id}/activate"]["patch"], "Admin Products");
    paths["/api/v1/admin/products/{id}/activate"]["patch"]["security"] = bearerSecurity();
    addPathParameter(paths["/api/v1/admin/products/{id}/activate"]["patch"], "id", "Id del producto");
    attachDefaultResponses(paths["/api/v1/admin/products/{id}/activate"]["patch"]);

    paths["/api/v1/admin/products/{id}/unavailable"]["patch"]["summary"] = "Marcar producto no disponible";
    addTag(paths["/api/v1/admin/products/{id}/unavailable"]["patch"], "Admin Products");
    paths["/api/v1/admin/products/{id}/unavailable"]["patch"]["security"] = bearerSecurity();
    addPathParameter(paths["/api/v1/admin/products/{id}/unavailable"]["patch"], "id", "Id del producto");
    attachDefaultResponses(paths["/api/v1/admin/products/{id}/unavailable"]["patch"]);

    paths["/api/v1/admin/products/{id}/deactivate"]["patch"]["summary"] = "Desactivar producto";
    addTag(paths["/api/v1/admin/products/{id}/deactivate"]["patch"], "Admin Products");
    paths["/api/v1/admin/products/{id}/deactivate"]["patch"]["security"] = bearerSecurity();
    addPathParameter(paths["/api/v1/admin/products/{id}/deactivate"]["patch"], "id", "Id del producto");
    attachDefaultResponses(paths["/api/v1/admin/products/{id}/deactivate"]["patch"]);

    paths["/api/v1/admin/addons"]["get"]["summary"] = "Listar addons";
    addTag(paths["/api/v1/admin/addons"]["get"], "Admin Addons");
    paths["/api/v1/admin/addons"]["get"]["security"] = bearerSecurity();
    addQueryParameter(paths["/api/v1/admin/addons"]["get"], "businessId", "Contexto de cafeteria para SUPER_ADMIN");
    addQueryParameter(paths["/api/v1/admin/addons"]["get"], "includeInactive", "Incluir addons inactivos");
    attachDefaultResponses(paths["/api/v1/admin/addons"]["get"]);

    paths["/api/v1/admin/addons"]["post"]["summary"] = "Crear addon";
    addTag(paths["/api/v1/admin/addons"]["post"], "Admin Addons");
    paths["/api/v1/admin/addons"]["post"]["security"] = bearerSecurity();
    addJsonRequestBody(paths["/api/v1/admin/addons"]["post"], {{"businessId", "integer"}, {"name", "string"}, {"price", "number"}}, {"name", "price"});
    attachDefaultResponses(paths["/api/v1/admin/addons"]["post"]);

    paths["/api/v1/admin/products/{productId}/addons/{addonId}"]["post"]["summary"] = "Asignar addon a producto";
    addTag(paths["/api/v1/admin/products/{productId}/addons/{addonId}"]["post"], "Admin Addons");
    paths["/api/v1/admin/products/{productId}/addons/{addonId}"]["post"]["security"] = bearerSecurity();
    addPathParameter(paths["/api/v1/admin/products/{productId}/addons/{addonId}"]["post"], "productId", "Id del producto");
    addPathParameter(paths["/api/v1/admin/products/{productId}/addons/{addonId}"]["post"], "addonId", "Id del addon");
    attachDefaultResponses(paths["/api/v1/admin/products/{productId}/addons/{addonId}"]["post"]);

    paths["/api/v1/admin/orders"]["get"]["summary"] = "Listar pedidos admin";
    addTag(paths["/api/v1/admin/orders"]["get"], "Admin Orders");
    paths["/api/v1/admin/orders"]["get"]["security"] = bearerSecurity();
    addQueryParameter(paths["/api/v1/admin/orders"]["get"], "businessId", "Contexto de cafeteria para SUPER_ADMIN");
    attachDefaultResponses(paths["/api/v1/admin/orders"]["get"]);

    paths["/api/v1/admin/orders/history"]["get"]["summary"] = "Historial de pedidos admin";
    addTag(paths["/api/v1/admin/orders/history"]["get"], "Admin Orders");
    paths["/api/v1/admin/orders/history"]["get"]["security"] = bearerSecurity();
    addQueryParameter(paths["/api/v1/admin/orders/history"]["get"], "businessId", "Contexto de cafeteria para SUPER_ADMIN");
    attachDefaultResponses(paths["/api/v1/admin/orders/history"]["get"]);

    paths["/api/v1/admin/orders/{orderId}/cancel"]["patch"]["summary"] = "Cancelar pedido";
    addTag(paths["/api/v1/admin/orders/{orderId}/cancel"]["patch"], "Admin Orders");
    paths["/api/v1/admin/orders/{orderId}/cancel"]["patch"]["security"] = bearerSecurity();
    addPathParameter(paths["/api/v1/admin/orders/{orderId}/cancel"]["patch"], "orderId", "Id del pedido");
    attachDefaultResponses(paths["/api/v1/admin/orders/{orderId}/cancel"]["patch"]);

    paths["/api/v1/admin/cashier/orders/search"]["get"]["summary"] = "Buscar pedidos para caja";
    addTag(paths["/api/v1/admin/cashier/orders/search"]["get"], "Cashier");
    paths["/api/v1/admin/cashier/orders/search"]["get"]["security"] = bearerSecurity();
    addQueryParameter(paths["/api/v1/admin/cashier/orders/search"]["get"], "businessId", "Contexto de cafeteria para SUPER_ADMIN");
    addQueryParameter(paths["/api/v1/admin/cashier/orders/search"]["get"], "customerName", "Nombre del cliente");
    addQueryParameter(paths["/api/v1/admin/cashier/orders/search"]["get"], "tableNumber", "Numero de mesa");
    addQueryParameter(paths["/api/v1/admin/cashier/orders/search"]["get"], "status", "Estado del pedido");
    attachDefaultResponses(paths["/api/v1/admin/cashier/orders/search"]["get"]);

    paths["/api/v1/admin/cashier/orders/{orderId}/pay"]["post"]["summary"] = "Pagar pedido";
    addTag(paths["/api/v1/admin/cashier/orders/{orderId}/pay"]["post"], "Cashier");
    paths["/api/v1/admin/cashier/orders/{orderId}/pay"]["post"]["security"] = bearerSecurity();
    addPathParameter(paths["/api/v1/admin/cashier/orders/{orderId}/pay"]["post"], "orderId", "Id del pedido");
    addJsonRequestBody(paths["/api/v1/admin/cashier/orders/{orderId}/pay"]["post"], {{"businessId", "integer"}, {"amount", "number"}}, {"amount"});
    attachDefaultResponses(paths["/api/v1/admin/cashier/orders/{orderId}/pay"]["post"]);

    paths["/api/v1/admin/products/{productId}/image"]["post"]["summary"] = "Subir imagen principal de producto";
    addTag(paths["/api/v1/admin/products/{productId}/image"]["post"], "Admin Product Images");
    paths["/api/v1/admin/products/{productId}/image"]["post"]["security"] = bearerSecurity();
    addPathParameter(paths["/api/v1/admin/products/{productId}/image"]["post"], "productId", "Id del producto");
    addQueryParameter(paths["/api/v1/admin/products/{productId}/image"]["post"], "businessId", "Contexto de cafeteria para SUPER_ADMIN");
    addMultipartRequestBody(paths["/api/v1/admin/products/{productId}/image"]["post"]);
    attachDefaultResponses(paths["/api/v1/admin/products/{productId}/image"]["post"]);

    paths["/api/v1/admin/products/{productId}/image"]["patch"]["summary"] = "Reemplazar imagen principal de producto";
    addTag(paths["/api/v1/admin/products/{productId}/image"]["patch"], "Admin Product Images");
    paths["/api/v1/admin/products/{productId}/image"]["patch"]["security"] = bearerSecurity();
    addPathParameter(paths["/api/v1/admin/products/{productId}/image"]["patch"], "productId", "Id del producto");
    addQueryParameter(paths["/api/v1/admin/products/{productId}/image"]["patch"], "businessId", "Contexto de cafeteria para SUPER_ADMIN");
    addMultipartRequestBody(paths["/api/v1/admin/products/{productId}/image"]["patch"]);
    attachDefaultResponses(paths["/api/v1/admin/products/{productId}/image"]["patch"]);

    paths["/api/v1/admin/products/{productId}/image"]["delete"]["summary"] = "Eliminar metadata de imagen principal";
    addTag(paths["/api/v1/admin/products/{productId}/image"]["delete"], "Admin Product Images");
    paths["/api/v1/admin/products/{productId}/image"]["delete"]["security"] = bearerSecurity();
    addPathParameter(paths["/api/v1/admin/products/{productId}/image"]["delete"], "productId", "Id del producto");
    addQueryParameter(paths["/api/v1/admin/products/{productId}/image"]["delete"], "businessId", "Contexto de cafeteria para SUPER_ADMIN");
    addQueryParameter(paths["/api/v1/admin/products/{productId}/image"]["delete"], "deleteFile", "Eliminar tambien el archivo fisico");
    attachDefaultResponses(paths["/api/v1/admin/products/{productId}/image"]["delete"]);

    callback(drogon::HttpResponse::newHttpJsonResponse(root));
}
}  // namespace starcafe::interfaces::rest
