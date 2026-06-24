#include "infrastructure/security/auth_filters.h"

#include <drogon/drogon.h>

namespace starcafe::infrastructure::security
{
namespace
{
JwtService *gJwtService = nullptr;

drogon::HttpResponsePtr unauthorized(const std::string &message)
{
    Json::Value body;
    body["success"] = false;
    body["message"] = message;
    auto response = drogon::HttpResponse::newHttpJsonResponse(body);
    response->setStatusCode(drogon::k401Unauthorized);
    return response;
}
}  // namespace

void configureJwt(JwtService *jwtService) { gJwtService = jwtService; }

std::optional<JwtClaims> parseBearerClaims(const drogon::HttpRequestPtr &request)
{
    if (gJwtService == nullptr)
    {
        return std::nullopt;
    }
    const auto header = request->getHeader("Authorization");
    constexpr std::string_view prefix = "Bearer ";
    if (!header.starts_with(prefix))
    {
        return std::nullopt;
    }
    return gJwtService->verify(header.substr(prefix.size()));
}

void JwtAuthFilter::doFilter(const drogon::HttpRequestPtr &req,
                             drogon::FilterCallback &&fcb,
                             drogon::FilterChainCallback &&fccb)
{
    if (!parseBearerClaims(req).has_value())
    {
        return fcb(unauthorized("Invalid or missing bearer token"));
    }
    fccb();
}

void AdminFilter::doFilter(const drogon::HttpRequestPtr &req,
                           drogon::FilterCallback &&fcb,
                           drogon::FilterChainCallback &&fccb)
{
    const auto claims = parseBearerClaims(req);
    if (!claims.has_value())
    {
        return fcb(unauthorized("Invalid or missing bearer token"));
    }
    if (claims->role != "ADMIN")
    {
        return fcb(unauthorized("Admin role required"));
    }
    fccb();
}

void KitchenFilter::doFilter(const drogon::HttpRequestPtr &req,
                             drogon::FilterCallback &&fcb,
                             drogon::FilterChainCallback &&fccb)
{
    const auto claims = parseBearerClaims(req);
    if (!claims.has_value())
    {
        return fcb(unauthorized("Invalid or missing bearer token"));
    }
    if (claims->role != "KITCHEN" && claims->role != "ADMIN")
    {
        return fcb(unauthorized("Kitchen or admin role required"));
    }
    fccb();
}
}  // namespace starcafe::infrastructure::security
