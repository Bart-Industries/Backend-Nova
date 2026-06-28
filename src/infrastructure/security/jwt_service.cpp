#include "infrastructure/security/jwt_service.h"

#include <drogon/utils/Utilities.h>
#include <json/json.h>
#include <openssl/hmac.h>

#include <chrono>
#include <sstream>

namespace starcafe::infrastructure::security
{
namespace
{
std::string base64UrlEncode(const std::string &input)
{
    auto encoded = drogon::utils::base64Encode(input);
    std::replace(encoded.begin(), encoded.end(), '+', '-');
    std::replace(encoded.begin(), encoded.end(), '/', '_');
    encoded.erase(std::remove(encoded.begin(), encoded.end(), '='), encoded.end());
    return encoded;
}

std::string base64UrlDecode(std::string input)
{
    std::replace(input.begin(), input.end(), '-', '+');
    std::replace(input.begin(), input.end(), '_', '/');
    while (input.size() % 4 != 0)
    {
        input.push_back('=');
    }
    return drogon::utils::base64Decode(input);
}

std::string hmacSha256(const std::string &data, const std::string &secret)
{
    unsigned int len = 0;
    unsigned char *digest = HMAC(EVP_sha256(),
                                 secret.data(),
                                 static_cast<int>(secret.size()),
                                 reinterpret_cast<const unsigned char *>(data.data()),
                                 static_cast<int>(data.size()),
                                 nullptr,
                                 &len);

    return std::string(reinterpret_cast<char *>(digest), len);
}
}  // namespace

JwtService::JwtService(std::string secret, std::int64_t expiresInSeconds)
    : secret_(std::move(secret)), expiresInSeconds_(expiresInSeconds)
{
}

std::string JwtService::createToken(std::int64_t userId,
                                    std::optional<std::int64_t> businessId,
                                    const std::string &name,
                                    const std::string &email,
                                    domain::UserRole role) const
{
    Json::Value header;
    header["alg"] = "HS256";
    header["typ"] = "JWT";

    const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    Json::Value payload;
    payload["sub"] = Json::Int64(userId);
    if (businessId.has_value())
    {
        payload["businessId"] = Json::Int64(*businessId);
    }
    payload["name"] = name;
    payload["email"] = email;
    payload["role"] = domain::toString(role);
    payload["exp"] = Json::Int64(now + expiresInSeconds_);

    Json::StreamWriterBuilder builder;
    const auto headerPart = base64UrlEncode(Json::writeString(builder, header));
    const auto payloadPart = base64UrlEncode(Json::writeString(builder, payload));
    const auto signedData = headerPart + "." + payloadPart;
    const auto signature = base64UrlEncode(hmacSha256(signedData, secret_));

    return signedData + "." + signature;
}

std::optional<JwtClaims> JwtService::verify(const std::string &token) const
{
    const auto first = token.find('.');
    const auto second = token.find('.', first + 1);
    if (first == std::string::npos || second == std::string::npos)
    {
        return std::nullopt;
    }

    const auto headerPart = token.substr(0, first);
    const auto payloadPart = token.substr(first + 1, second - first - 1);
    const auto signaturePart = token.substr(second + 1);

    const auto signedData = headerPart + "." + payloadPart;
    if (base64UrlEncode(hmacSha256(signedData, secret_)) != signaturePart)
    {
        return std::nullopt;
    }

    Json::CharReaderBuilder builder;
    Json::Value payload;
    std::string errors;
    std::istringstream iss(base64UrlDecode(payloadPart));
    if (!Json::parseFromStream(builder, iss, &payload, &errors))
    {
        return std::nullopt;
    }

    const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    if (payload["exp"].asInt64() < now)
    {
        return std::nullopt;
    }

    JwtClaims claims;
    claims.userId = payload["sub"].asInt64();
    if (!payload["businessId"].isNull())
    {
        claims.businessId = payload["businessId"].asInt64();
    }
    claims.name = payload["name"].asString();
    claims.email = payload["email"].asString();
    claims.role = payload["role"].asString();
    claims.exp = payload["exp"].asInt64();
    return claims;
}
}  // namespace starcafe::infrastructure::security
