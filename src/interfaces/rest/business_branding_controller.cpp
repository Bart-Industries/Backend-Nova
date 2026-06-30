#include "interfaces/rest/business_branding_controller.h"

#include "domain/businesses/theme_catalog.h"
#include "domain/common/errors.h"
#include "infrastructure/security/auth_filters.h"
#include "interfaces/rest/service_registry.h"

#include <drogon/MultiPart.h>
#include <drogon/utils/Utilities.h>
#include <filesystem>
#include <json/json.h>

namespace starcafe::interfaces::rest
{
namespace
{
std::string mimeTypeFromContentType(drogon::ContentType contentType)
{
    switch (contentType)
    {
    case drogon::CT_IMAGE_PNG:
        return "image/png";
    case drogon::CT_IMAGE_JPG:
        return "image/jpeg";
    case drogon::CT_IMAGE_WEBP:
        return "image/webp";
    default:
        return "";
    }
}

drogon::HttpResponsePtr jsonResponse(bool success, const Json::Value &data, const std::string &message = "", drogon::HttpStatusCode code = drogon::k200OK)
{
    Json::Value body;
    body["success"] = success;
    if (success)
    {
        body["data"] = data;
    }
    else
    {
        body["message"] = message;
    }
    auto response = drogon::HttpResponse::newHttpJsonResponse(body);
    response->setStatusCode(code);
    return response;
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

std::int64_t resolveBusinessId(const drogon::HttpRequestPtr &req)
{
    const auto claims = infrastructure::security::parseBearerClaims(req);
    if (!claims.has_value())
    {
        throw domain::DomainError("Invalid token");
    }
    if (!claims->businessId.has_value())
    {
        throw domain::DomainError("Business context is required");
    }
    return *claims->businessId;
}

Json::Value themeToJson(const domain::businesses::ThemeOption &theme)
{
    Json::Value value;
    value["key"] = std::string(theme.key);
    value["primaryColor"] = std::string(theme.primaryColor);
    return value;
}

Json::Value businessToJson(const domain::businesses::Business &business)
{
    Json::Value value;
    value["id"] = Json::Int64(business.id);
    value["name"] = business.name;
    value["slug"] = business.slug;
    value["logoUrl"] = business.logoUrl;
    value["primaryColor"] = business.primaryColor;
    value["isActive"] = business.isActive;
    value["createdAt"] = business.createdAt;
    if (const auto themeKey = domain::businesses::themeKeyFromColor(business.primaryColor); themeKey.has_value())
    {
        value["themeKey"] = std::string(*themeKey);
    }
    else
    {
        value["themeKey"] = Json::nullValue;
    }
    return value;
}

Json::Value brandingPayload(const domain::businesses::Business &business)
{
    Json::Value data;
    data["business"] = businessToJson(business);
    for (const auto &theme : domain::businesses::kThemeCatalog)
    {
        data["availableThemes"].append(themeToJson(theme));
    }
    return data;
}

application::UploadBusinessLogoCommand mapUploadCommand(const drogon::HttpRequestPtr &req)
{
    drogon::MultiPartParser parser;
    if (parser.parse(req) != 0)
    {
        throw domain::DomainError("Invalid multipart/form-data request");
    }

    const auto &files = parser.getFiles();
    if (files.empty())
    {
        throw domain::DomainError("Logo file is required");
    }

    const auto &file = files.front();
    if (file.getItemName() != "logo")
    {
        throw domain::DomainError("Multipart field 'logo' is required");
    }

    application::UploadBusinessLogoCommand command;
    command.originalFileName = file.getFileName();
    command.mimeType = mimeTypeFromContentType(file.getContentType());
    if (command.mimeType.empty())
    {
        throw domain::DomainError("Solo se permiten imagenes PNG, JPG, JPEG o WEBP");
    }
    command.fileSize = static_cast<std::int64_t>(file.fileLength());

    const auto tempDirectory = std::filesystem::temp_directory_path();
    const auto tempPath = tempDirectory / ("nova-business-logo-" + drogon::utils::getUuid() + ".tmp");
    if (file.saveAs(tempPath.string()) != 0)
    {
        throw domain::DomainError("Could not persist uploaded logo");
    }
    command.tempFilePath = tempPath.string();
    return command;
}
}  // namespace

void BusinessBrandingController::getCurrentBusiness(const drogon::HttpRequestPtr &req,
                                                    std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    try
    {
        callback(jsonResponse(true, brandingPayload(services().getBusiness->execute(resolveBusinessId(req)))));
    }
    catch (const domain::DomainError &error)
    {
        callback(jsonResponse(false, Json::nullValue, error.what(), drogon::k400BadRequest));
    }
    catch (const std::exception &error)
    {
        callback(jsonResponse(false, Json::nullValue, error.what(), drogon::k500InternalServerError));
    }
}

void BusinessBrandingController::updateTheme(const drogon::HttpRequestPtr &req,
                                             std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    try
    {
        const auto &json = body(req);
        application::UpdateBusinessThemeCommand command;
        command.themeKey = json.get("themeKey", "").asString();
        callback(jsonResponse(true, brandingPayload(services().updateBusinessTheme->execute(resolveBusinessId(req), command))));
    }
    catch (const domain::DomainError &error)
    {
        callback(jsonResponse(false, Json::nullValue, error.what(), drogon::k400BadRequest));
    }
    catch (const std::exception &error)
    {
        callback(jsonResponse(false, Json::nullValue, error.what(), drogon::k500InternalServerError));
    }
}

void BusinessBrandingController::updateLogo(const drogon::HttpRequestPtr &req,
                                            std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    try
    {
        callback(jsonResponse(true,
                              brandingPayload(services().updateBusinessLogo->execute(resolveBusinessId(req), mapUploadCommand(req)))));
    }
    catch (const domain::DomainError &error)
    {
        callback(jsonResponse(false, Json::nullValue, error.what(), drogon::k400BadRequest));
    }
    catch (const std::exception &error)
    {
        callback(jsonResponse(false, Json::nullValue, error.what(), drogon::k500InternalServerError));
    }
}
}  // namespace starcafe::interfaces::rest
