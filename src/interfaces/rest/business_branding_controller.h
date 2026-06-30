#pragma once

#include <drogon/HttpController.h>

namespace starcafe::interfaces::rest
{
class BusinessBrandingController : public drogon::HttpController<BusinessBrandingController>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(BusinessBrandingController::getCurrentBusiness, "/api/v1/admin/business", drogon::Get, "starcafe::infrastructure::security::AdminFilter");
    ADD_METHOD_TO(BusinessBrandingController::updateTheme, "/api/v1/admin/business/theme", drogon::Patch, "starcafe::infrastructure::security::AdminFilter");
    ADD_METHOD_TO(BusinessBrandingController::updateLogo, "/api/v1/admin/business/logo", drogon::Patch, "starcafe::infrastructure::security::AdminFilter");
    METHOD_LIST_END

    void getCurrentBusiness(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void updateTheme(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void updateLogo(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
};
}  // namespace starcafe::interfaces::rest
