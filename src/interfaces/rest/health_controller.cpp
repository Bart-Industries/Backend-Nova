#include "interfaces/rest/docs_controller.h"

#include "interfaces/rest/service_registry.h"

#include <drogon/HttpResponse.h>

namespace starcafe::interfaces::rest
{
namespace
{
drogon::HttpResponsePtr jsonResponse(const Json::Value &value, drogon::HttpStatusCode code = drogon::k200OK)
{
    auto response = drogon::HttpResponse::newHttpJsonResponse(value);
    response->setStatusCode(code);
    return response;
}
}  // namespace

void DocsController::health(const drogon::HttpRequestPtr &, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    Json::Value body;
    body["success"] = true;
    body["data"]["status"] = "ok";
    body["data"]["service"] = "nova-backend";
    callback(jsonResponse(body));
}

void DocsController::ready(const drogon::HttpRequestPtr &, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    Json::Value body;
    body["success"] = true;
    body["data"]["status"] = "ready";
    body["data"]["service"] = "nova-backend";

    try
    {
        if (!services().dbClient)
        {
            throw std::runtime_error("Database client is not configured");
        }

        services().dbClient->execSqlSync("select 1");
        body["data"]["database"] = "ok";
        callback(jsonResponse(body));
    }
    catch (const std::exception &error)
    {
        body["success"] = false;
        body["message"] = error.what();
        body["data"]["status"] = "not_ready";
        body["data"]["database"] = "error";
        callback(jsonResponse(body, drogon::k503ServiceUnavailable));
    }
}
}  // namespace starcafe::interfaces::rest
