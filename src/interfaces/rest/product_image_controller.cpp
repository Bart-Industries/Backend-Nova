#include "interfaces/rest/product_image_controller.h"

#include "domain/common/errors.h"
#include "interfaces/rest/service_registry.h"
#include "infrastructure/storage/file_storage_service.h"

#include <drogon/MultiPart.h>
#include <filesystem>

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

Json::Value productImageToJson(const domain::menu::ProductImage &image)
{
    Json::Value data;
    data["id"] = Json::Int64(image.id);
    data["product_id"] = Json::Int64(image.productId);
    data["file_name"] = image.fileName;
    data["mime_type"] = image.mimeType;
    data["file_size"] = Json::Int64(image.fileSize);
    data["url"] = services().publicProductFilesBaseUrl + "/" + image.fileName;
    return data;
}

application::UploadProductImageCommand mapUploadCommand(const drogon::HttpRequestPtr &req, const std::string &productId)
{
    drogon::MultiPartParser parser;
    if (parser.parse(req) != 0)
    {
        throw domain::DomainError("Invalid multipart/form-data request");
    }

    const auto &files = parser.getFiles();
    if (files.empty())
    {
        throw domain::DomainError("Image file is required");
    }

    const auto &file = files.front();
    if (file.getItemName() != "image")
    {
        throw domain::DomainError("Multipart field 'image' is required");
    }

    application::UploadProductImageCommand command;
    command.productId = std::stoll(productId);
    command.originalFileName = file.getFileName();
    command.mimeType = mimeTypeFromContentType(file.getContentType());
    if (command.mimeType.empty())
    {
        throw domain::DomainError("Solo se permiten imagenes PNG, JPG, JPEG o WEBP");
    }
    command.fileSize = static_cast<std::int64_t>(file.fileLength());
    const auto tempDirectory = std::filesystem::temp_directory_path();
    const auto tempPath = tempDirectory / ("starcafe-upload-" + drogon::utils::getUuid() + ".tmp");
    if (file.saveAs(tempPath.string()) != 0)
    {
        throw domain::DomainError("Could not persist uploaded image");
    }
    command.tempFilePath = tempPath.string();
    return command;
}
}  // namespace

void ProductImageController::uploadMainImage(const drogon::HttpRequestPtr &req,
                                             std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                                             std::string productId)
{
    try
    {
        callback(jsonResponse(true, productImageToJson(services().uploadProductImage->execute(mapUploadCommand(req, productId))), "", drogon::k201Created));
    }
    catch (const domain::DomainError &error)
    {
        callback(jsonResponse(false, Json::nullValue, error.what()));
    }
}

void ProductImageController::replaceMainImage(const drogon::HttpRequestPtr &req,
                                              std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                                              std::string productId)
{
    try
    {
        callback(jsonResponse(true, productImageToJson(services().replaceProductImage->execute(mapUploadCommand(req, productId)))));
    }
    catch (const domain::DomainError &error)
    {
        callback(jsonResponse(false, Json::nullValue, error.what()));
    }
}

void ProductImageController::deleteMainImage(const drogon::HttpRequestPtr &req,
                                             std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                                             std::string productId)
{
    try
    {
        const bool deletePhysicalFile = req->getParameter("deleteFile") == "true";
        services().deleteProductImage->execute(std::stoll(productId), deletePhysicalFile);
        callback(jsonResponse(true, Json::Value("Product image removed")));
    }
    catch (const domain::DomainError &error)
    {
        callback(jsonResponse(false, Json::nullValue, error.what()));
    }
}

void ProductImageController::serveImage(const drogon::HttpRequestPtr &,
                                        std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                                        std::string fileName)
{
    try
    {
        const auto absolutePath = services().fileStorageService->resolvePublicFile(fileName);
        callback(drogon::HttpResponse::newFileResponse(absolutePath));
    }
    catch (const domain::DomainError &error)
    {
        callback(jsonResponse(false, Json::nullValue, error.what(), drogon::k400BadRequest));
    }
}
}  // namespace starcafe::interfaces::rest
