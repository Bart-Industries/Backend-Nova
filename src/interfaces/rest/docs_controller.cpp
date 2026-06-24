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

void addMultipartRequestBody(Json::Value &operation)
{
    Json::Value schema;
    schema["type"] = "object";
    schema["required"].append("image");
    schema["properties"]["image"]["type"] = "string";
    schema["properties"]["image"]["format"] = "binary";
    operation["requestBody"]["required"] = true;
    operation["requestBody"]["content"]["multipart/form-data"]["schema"] = schema;
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
}  // namespace

void DocsController::swaggerUi(const drogon::HttpRequestPtr &, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    static const std::string html = R"HTML(
<!doctype html>
<html lang="en">
  <head>
    <meta charset="utf-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1" />
    <title>StarCafe API Docs</title>
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
        persistAuthorization: true
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
    root["info"]["title"] = "StarCafe API";
    root["info"]["version"] = "1.0.0";
    root["info"]["description"] = "Documentacion para pruebas del backend de StarCafe.";

    root["components"]["securitySchemes"]["bearerAuth"]["type"] = "http";
    root["components"]["securitySchemes"]["bearerAuth"]["scheme"] = "bearer";
    root["components"]["securitySchemes"]["bearerAuth"]["bearerFormat"] = "JWT";

    auto &paths = root["paths"];

    paths["/api/v1/auth/register"]["post"]["summary"] = "Registrar usuario";
    addJsonRequestBody(paths["/api/v1/auth/register"]["post"],
                       {{"name", "string"}, {"email", "string"}, {"password", "string"}, {"role", "string"}},
                       {"name", "email", "password", "role"});
    attachDefaultResponses(paths["/api/v1/auth/register"]["post"]);

    paths["/api/v1/auth/login"]["post"]["summary"] = "Login";
    addJsonRequestBody(paths["/api/v1/auth/login"]["post"], {{"email", "string"}, {"password", "string"}}, {"email", "password"});
    attachDefaultResponses(paths["/api/v1/auth/login"]["post"]);

    paths["/api/v1/auth/me"]["get"]["summary"] = "Usuario autenticado";
    paths["/api/v1/auth/me"]["get"]["security"] = bearerSecurity();
    attachDefaultResponses(paths["/api/v1/auth/me"]["get"]);

    paths["/api/v1/tables/qr/{qrToken}"]["get"]["summary"] = "Obtener mesa por QR";
    addPathParameter(paths["/api/v1/tables/qr/{qrToken}"]["get"], "qrToken", "Token del QR");
    attachDefaultResponses(paths["/api/v1/tables/qr/{qrToken}"]["get"]);

    paths["/api/v1/public/menu"]["get"]["summary"] = "Listar menu publico";
    attachDefaultResponses(paths["/api/v1/public/menu"]["get"]);

    paths["/api/v1/public/tables/{qrToken}/orders"]["post"]["summary"] = "Crear pedido desde mesa";
    addPathParameter(paths["/api/v1/public/tables/{qrToken}/orders"]["post"], "qrToken", "Token del QR");
    addJsonRequestBody(paths["/api/v1/public/tables/{qrToken}/orders"]["post"],
                       {{"customerName", "string"}, {"items", "array"}},
                       {"customerName", "items"});
    attachDefaultResponses(paths["/api/v1/public/tables/{qrToken}/orders"]["post"]);

    paths["/api/v1/public/orders/{orderId}/status"]["get"]["summary"] = "Estado de pedido";
    addPathParameter(paths["/api/v1/public/orders/{orderId}/status"]["get"], "orderId", "Id del pedido");
    attachDefaultResponses(paths["/api/v1/public/orders/{orderId}/status"]["get"]);

    paths["/api/v1/kitchen/orders"]["get"]["summary"] = "Pedidos activos de cocina";
    paths["/api/v1/kitchen/orders"]["get"]["security"] = bearerSecurity();
    attachDefaultResponses(paths["/api/v1/kitchen/orders"]["get"]);

    paths["/api/v1/kitchen/orders/{orderId}/preparing"]["patch"]["summary"] = "Mover pedido a PREPARING";
    paths["/api/v1/kitchen/orders/{orderId}/preparing"]["patch"]["security"] = bearerSecurity();
    addPathParameter(paths["/api/v1/kitchen/orders/{orderId}/preparing"]["patch"], "orderId", "Id del pedido");
    attachDefaultResponses(paths["/api/v1/kitchen/orders/{orderId}/preparing"]["patch"]);

    paths["/api/v1/kitchen/order-items/{itemId}/ready"]["patch"]["summary"] = "Marcar item como READY";
    paths["/api/v1/kitchen/order-items/{itemId}/ready"]["patch"]["security"] = bearerSecurity();
    addPathParameter(paths["/api/v1/kitchen/order-items/{itemId}/ready"]["patch"], "itemId", "Id del item");
    attachDefaultResponses(paths["/api/v1/kitchen/order-items/{itemId}/ready"]["patch"]);

    paths["/api/v1/kitchen/orders/{orderId}/ready"]["patch"]["summary"] = "Marcar pedido como READY";
    paths["/api/v1/kitchen/orders/{orderId}/ready"]["patch"]["security"] = bearerSecurity();
    addPathParameter(paths["/api/v1/kitchen/orders/{orderId}/ready"]["patch"], "orderId", "Id del pedido");
    attachDefaultResponses(paths["/api/v1/kitchen/orders/{orderId}/ready"]["patch"]);

    paths["/api/v1/kitchen/orders/history"]["get"]["summary"] = "Historial de cocina";
    paths["/api/v1/kitchen/orders/history"]["get"]["security"] = bearerSecurity();
    attachDefaultResponses(paths["/api/v1/kitchen/orders/history"]["get"]);

    paths["/api/v1/admin/users"]["get"]["summary"] = "Listar usuarios";
    paths["/api/v1/admin/users"]["get"]["security"] = bearerSecurity();
    attachDefaultResponses(paths["/api/v1/admin/users"]["get"]);

    paths["/api/v1/admin/users"]["post"]["summary"] = "Crear usuario";
    paths["/api/v1/admin/users"]["post"]["security"] = bearerSecurity();
    addJsonRequestBody(paths["/api/v1/admin/users"]["post"],
                       {{"name", "string"}, {"email", "string"}, {"password", "string"}, {"role", "string"}},
                       {"name", "email", "password", "role"});
    attachDefaultResponses(paths["/api/v1/admin/users"]["post"]);

    paths["/api/v1/admin/users/{id}/deactivate"]["patch"]["summary"] = "Desactivar usuario";
    paths["/api/v1/admin/users/{id}/deactivate"]["patch"]["security"] = bearerSecurity();
    addPathParameter(paths["/api/v1/admin/users/{id}/deactivate"]["patch"], "id", "Id del usuario");
    attachDefaultResponses(paths["/api/v1/admin/users/{id}/deactivate"]["patch"]);

    paths["/api/v1/admin/tables"]["get"]["summary"] = "Listar mesas";
    paths["/api/v1/admin/tables"]["get"]["security"] = bearerSecurity();
    attachDefaultResponses(paths["/api/v1/admin/tables"]["get"]);

    paths["/api/v1/admin/tables"]["post"]["summary"] = "Crear mesa";
    paths["/api/v1/admin/tables"]["post"]["security"] = bearerSecurity();
    addJsonRequestBody(paths["/api/v1/admin/tables"]["post"], {{"tableNumber", "integer"}}, {"tableNumber"});
    attachDefaultResponses(paths["/api/v1/admin/tables"]["post"]);

    paths["/api/v1/admin/tables/{id}/deactivate"]["patch"]["summary"] = "Desactivar mesa";
    paths["/api/v1/admin/tables/{id}/deactivate"]["patch"]["security"] = bearerSecurity();
    addPathParameter(paths["/api/v1/admin/tables/{id}/deactivate"]["patch"], "id", "Id de la mesa");
    attachDefaultResponses(paths["/api/v1/admin/tables/{id}/deactivate"]["patch"]);

    paths["/api/v1/admin/categories"]["get"]["summary"] = "Listar categorias";
    paths["/api/v1/admin/categories"]["get"]["security"] = bearerSecurity();
    attachDefaultResponses(paths["/api/v1/admin/categories"]["get"]);

    paths["/api/v1/admin/categories"]["post"]["summary"] = "Crear categoria";
    paths["/api/v1/admin/categories"]["post"]["security"] = bearerSecurity();
    addJsonRequestBody(paths["/api/v1/admin/categories"]["post"], {{"name", "string"}, {"description", "string"}}, {"name"});
    attachDefaultResponses(paths["/api/v1/admin/categories"]["post"]);

    paths["/api/v1/admin/products"]["get"]["summary"] = "Listar productos";
    paths["/api/v1/admin/products"]["get"]["security"] = bearerSecurity();
    attachDefaultResponses(paths["/api/v1/admin/products"]["get"]);

    paths["/api/v1/admin/products"]["post"]["summary"] = "Crear producto";
    paths["/api/v1/admin/products"]["post"]["security"] = bearerSecurity();
    addJsonRequestBody(paths["/api/v1/admin/products"]["post"],
                       {{"categoryId", "integer"}, {"name", "string"}, {"description", "string"}, {"price", "number"}},
                       {"categoryId", "name", "price"});
    attachDefaultResponses(paths["/api/v1/admin/products"]["post"]);

    paths["/api/v1/admin/products/{id}"]["patch"]["summary"] = "Actualizar producto";
    paths["/api/v1/admin/products/{id}"]["patch"]["security"] = bearerSecurity();
    addPathParameter(paths["/api/v1/admin/products/{id}"]["patch"], "id", "Id del producto");
    addJsonRequestBody(paths["/api/v1/admin/products/{id}"]["patch"],
                       {{"categoryId", "integer"}, {"name", "string"}, {"description", "string"}, {"price", "number"}, {"isAvailable", "boolean"}});
    attachDefaultResponses(paths["/api/v1/admin/products/{id}"]["patch"]);

    paths["/api/v1/admin/products/{id}/unavailable"]["patch"]["summary"] = "Marcar producto no disponible";
    paths["/api/v1/admin/products/{id}/unavailable"]["patch"]["security"] = bearerSecurity();
    addPathParameter(paths["/api/v1/admin/products/{id}/unavailable"]["patch"], "id", "Id del producto");
    attachDefaultResponses(paths["/api/v1/admin/products/{id}/unavailable"]["patch"]);

    paths["/api/v1/admin/products/{id}/deactivate"]["patch"]["summary"] = "Desactivar producto";
    paths["/api/v1/admin/products/{id}/deactivate"]["patch"]["security"] = bearerSecurity();
    addPathParameter(paths["/api/v1/admin/products/{id}/deactivate"]["patch"], "id", "Id del producto");
    attachDefaultResponses(paths["/api/v1/admin/products/{id}/deactivate"]["patch"]);

    paths["/api/v1/admin/addons"]["get"]["summary"] = "Listar addons";
    paths["/api/v1/admin/addons"]["get"]["security"] = bearerSecurity();
    attachDefaultResponses(paths["/api/v1/admin/addons"]["get"]);

    paths["/api/v1/admin/addons"]["post"]["summary"] = "Crear addon";
    paths["/api/v1/admin/addons"]["post"]["security"] = bearerSecurity();
    addJsonRequestBody(paths["/api/v1/admin/addons"]["post"], {{"name", "string"}, {"price", "number"}}, {"name", "price"});
    attachDefaultResponses(paths["/api/v1/admin/addons"]["post"]);

    paths["/api/v1/admin/products/{productId}/addons/{addonId}"]["post"]["summary"] = "Asignar addon a producto";
    paths["/api/v1/admin/products/{productId}/addons/{addonId}"]["post"]["security"] = bearerSecurity();
    addPathParameter(paths["/api/v1/admin/products/{productId}/addons/{addonId}"]["post"], "productId", "Id del producto");
    addPathParameter(paths["/api/v1/admin/products/{productId}/addons/{addonId}"]["post"], "addonId", "Id del addon");
    attachDefaultResponses(paths["/api/v1/admin/products/{productId}/addons/{addonId}"]["post"]);

    paths["/api/v1/admin/orders"]["get"]["summary"] = "Listar pedidos admin";
    paths["/api/v1/admin/orders"]["get"]["security"] = bearerSecurity();
    attachDefaultResponses(paths["/api/v1/admin/orders"]["get"]);

    paths["/api/v1/admin/orders/history"]["get"]["summary"] = "Historial de pedidos admin";
    paths["/api/v1/admin/orders/history"]["get"]["security"] = bearerSecurity();
    attachDefaultResponses(paths["/api/v1/admin/orders/history"]["get"]);

    paths["/api/v1/admin/orders/{orderId}/cancel"]["patch"]["summary"] = "Cancelar pedido";
    paths["/api/v1/admin/orders/{orderId}/cancel"]["patch"]["security"] = bearerSecurity();
    addPathParameter(paths["/api/v1/admin/orders/{orderId}/cancel"]["patch"], "orderId", "Id del pedido");
    attachDefaultResponses(paths["/api/v1/admin/orders/{orderId}/cancel"]["patch"]);

    paths["/api/v1/admin/cashier/orders/search"]["get"]["summary"] = "Buscar pedidos para caja";
    paths["/api/v1/admin/cashier/orders/search"]["get"]["security"] = bearerSecurity();
    paths["/api/v1/admin/cashier/orders/search"]["get"]["parameters"][0]["in"] = "query";
    paths["/api/v1/admin/cashier/orders/search"]["get"]["parameters"][0]["name"] = "customerName";
    paths["/api/v1/admin/cashier/orders/search"]["get"]["parameters"][0]["schema"]["type"] = "string";
    paths["/api/v1/admin/cashier/orders/search"]["get"]["parameters"][1]["in"] = "query";
    paths["/api/v1/admin/cashier/orders/search"]["get"]["parameters"][1]["name"] = "tableNumber";
    paths["/api/v1/admin/cashier/orders/search"]["get"]["parameters"][1]["schema"]["type"] = "string";
    paths["/api/v1/admin/cashier/orders/search"]["get"]["parameters"][2]["in"] = "query";
    paths["/api/v1/admin/cashier/orders/search"]["get"]["parameters"][2]["name"] = "status";
    paths["/api/v1/admin/cashier/orders/search"]["get"]["parameters"][2]["schema"]["type"] = "string";
    attachDefaultResponses(paths["/api/v1/admin/cashier/orders/search"]["get"]);

    paths["/api/v1/admin/cashier/orders/{orderId}/pay"]["post"]["summary"] = "Pagar pedido";
    paths["/api/v1/admin/cashier/orders/{orderId}/pay"]["post"]["security"] = bearerSecurity();
    addPathParameter(paths["/api/v1/admin/cashier/orders/{orderId}/pay"]["post"], "orderId", "Id del pedido");
    addJsonRequestBody(paths["/api/v1/admin/cashier/orders/{orderId}/pay"]["post"], {{"amount", "number"}}, {"amount"});
    attachDefaultResponses(paths["/api/v1/admin/cashier/orders/{orderId}/pay"]["post"]);

    paths["/api/v1/admin/products/{productId}/image"]["post"]["summary"] = "Subir imagen principal de producto";
    paths["/api/v1/admin/products/{productId}/image"]["post"]["security"] = bearerSecurity();
    addPathParameter(paths["/api/v1/admin/products/{productId}/image"]["post"], "productId", "Id del producto");
    addMultipartRequestBody(paths["/api/v1/admin/products/{productId}/image"]["post"]);
    attachDefaultResponses(paths["/api/v1/admin/products/{productId}/image"]["post"]);

    paths["/api/v1/admin/products/{productId}/image"]["patch"]["summary"] = "Reemplazar imagen principal de producto";
    paths["/api/v1/admin/products/{productId}/image"]["patch"]["security"] = bearerSecurity();
    addPathParameter(paths["/api/v1/admin/products/{productId}/image"]["patch"], "productId", "Id del producto");
    addMultipartRequestBody(paths["/api/v1/admin/products/{productId}/image"]["patch"]);
    attachDefaultResponses(paths["/api/v1/admin/products/{productId}/image"]["patch"]);

    paths["/api/v1/admin/products/{productId}/image"]["delete"]["summary"] = "Eliminar metadata de imagen principal";
    paths["/api/v1/admin/products/{productId}/image"]["delete"]["security"] = bearerSecurity();
    addPathParameter(paths["/api/v1/admin/products/{productId}/image"]["delete"], "productId", "Id del producto");
    attachDefaultResponses(paths["/api/v1/admin/products/{productId}/image"]["delete"]);

    paths["/api/v1/uploads/products/{fileName}"]["get"]["summary"] = "Servir imagen de producto";
    addPathParameter(paths["/api/v1/uploads/products/{fileName}"]["get"], "fileName", "Nombre del archivo");
    paths["/api/v1/uploads/products/{fileName}"]["get"]["responses"]["200"]["description"] = "Archivo";

    callback(drogon::HttpResponse::newHttpJsonResponse(root));
}
}  // namespace starcafe::interfaces::rest
