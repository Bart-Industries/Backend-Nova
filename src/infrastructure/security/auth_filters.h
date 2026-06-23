#pragma once

#include "infrastructure/security/jwt_service.h"

#include <drogon/HttpFilter.h>

namespace starcafe::infrastructure::security
{
void configureJwt(JwtService *jwtService);
std::optional<JwtClaims> parseBearerClaims(const drogon::HttpRequestPtr &request);

class JwtAuthFilter : public drogon::HttpFilter<JwtAuthFilter>
{
  public:
    void doFilter(const drogon::HttpRequestPtr &req,
                  drogon::FilterCallback &&fcb,
                  drogon::FilterChainCallback &&fccb) override;
};

class AdminFilter : public drogon::HttpFilter<AdminFilter>
{
  public:
    void doFilter(const drogon::HttpRequestPtr &req,
                  drogon::FilterCallback &&fcb,
                  drogon::FilterChainCallback &&fccb) override;
};

class KitchenFilter : public drogon::HttpFilter<KitchenFilter>
{
  public:
    void doFilter(const drogon::HttpRequestPtr &req,
                  drogon::FilterCallback &&fcb,
                  drogon::FilterChainCallback &&fccb) override;
};
}  // namespace starcafe::infrastructure::security
