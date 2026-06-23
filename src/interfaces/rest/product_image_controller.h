#pragma once

#include <drogon/HttpController.h>

namespace starcafe::interfaces::rest
{
class ProductImageController : public drogon::HttpController<ProductImageController>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(ProductImageController::uploadMainImage, "/api/v1/admin/products/{1}/image", drogon::Post, "starcafe::infrastructure::security::AdminFilter");
    ADD_METHOD_TO(ProductImageController::replaceMainImage, "/api/v1/admin/products/{1}/image", drogon::Patch, "starcafe::infrastructure::security::AdminFilter");
    ADD_METHOD_TO(ProductImageController::deleteMainImage, "/api/v1/admin/products/{1}/image", drogon::Delete, "starcafe::infrastructure::security::AdminFilter");
    ADD_METHOD_TO(ProductImageController::serveImage, "/api/v1/uploads/products/{1}", drogon::Get);
    METHOD_LIST_END

    void uploadMainImage(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string productId);
    void replaceMainImage(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string productId);
    void deleteMainImage(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string productId);
    void serveImage(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string fileName);
};
}  // namespace starcafe::interfaces::rest
