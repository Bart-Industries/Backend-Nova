#pragma once

#include <drogon/HttpController.h>

namespace starcafe::interfaces::rest
{
class DocsController : public drogon::HttpController<DocsController>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(DocsController::health, "/health", drogon::Get);
    ADD_METHOD_TO(DocsController::ready, "/ready", drogon::Get);
    ADD_METHOD_TO(DocsController::swaggerUi, "/docs", drogon::Get);
    ADD_METHOD_TO(DocsController::openApiJson, "/openapi.json", drogon::Get);
    METHOD_LIST_END

    void health(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void ready(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void swaggerUi(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void openApiJson(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
};
}  // namespace starcafe::interfaces::rest
