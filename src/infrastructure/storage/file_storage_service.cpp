#include "infrastructure/storage/file_storage_service.h"

#include "domain/common/errors.h"

#include <drogon/drogon.h>
#include <drogon/utils/Utilities.h>
#include <json/json.h>
#include <openssl/sha.h>

#include <array>
#include <chrono>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <iomanip>
#include <sstream>

namespace starcafe::infrastructure::storage
{
namespace fs = std::filesystem;

namespace
{
struct CommandResult
{
    int exitCode{};
    std::string output;
};

std::string hexSha1(const std::string &value)
{
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char *>(value.data()), value.size(), hash);

    std::ostringstream stream;
    stream << std::hex << std::setfill('0');
    for (unsigned char byte : hash)
    {
        stream << std::setw(2) << static_cast<int>(byte);
    }
    return stream.str();
}

std::string basenameFromPath(const std::string &path)
{
    const auto position = path.find_last_of("/\\");
    if (position == std::string::npos)
    {
        return path;
    }
    return path.substr(position + 1);
}

std::string shellQuote(const std::string &value)
{
    std::string escaped;
    escaped.reserve(value.size() + 2);
    escaped.push_back('"');
    for (const auto ch : value)
    {
        if (ch == '"')
        {
            escaped += "\\\"";
        }
        else
        {
            escaped.push_back(ch);
        }
    }
    escaped.push_back('"');
    return escaped;
}

CommandResult runCommand(const std::string &command)
{
#ifdef _WIN32
    FILE *pipe = _popen(command.c_str(), "r");
#else
    FILE *pipe = popen(command.c_str(), "r");
#endif
    if (!pipe)
    {
        throw domain::DomainError("Could not start curl process for Cloudinary");
    }

    std::array<char, 4096> buffer{};
    std::string output;
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr)
    {
        output += buffer.data();
    }

#ifdef _WIN32
    const int exitCode = _pclose(pipe);
#else
    const int exitCode = pclose(pipe);
#endif
    return {exitCode, output};
}

Json::Value parseJsonOrThrow(const std::string &payload)
{
    Json::CharReaderBuilder builder;
    JSONCPP_STRING errors;
    std::istringstream stream(payload);
    Json::Value json;
    if (!Json::parseFromStream(builder, stream, &json, &errors))
    {
        throw domain::DomainError(payload.empty() ? "Cloudinary returned an invalid response" : payload);
    }
    return json;
}

std::string extractCloudinaryError(const Json::Value &json)
{
    if (json.isMember("error"))
    {
        const auto &errorNode = json["error"];
        if (errorNode.isObject() && errorNode.isMember("message") && errorNode["message"].isString())
        {
            return errorNode["message"].asString();
        }
        if (errorNode.isString())
        {
            return errorNode.asString();
        }
    }
    if (json.isMember("message") && json["message"].isString())
    {
        return json["message"].asString();
    }
    return {};
}
}  // namespace

FileStorageService::FileStorageService(FileStorageOptions options) : options_(std::move(options))
{
    if (options_.cloudinaryCloudName.empty() || options_.cloudinaryApiKey.empty() || options_.cloudinaryApiSecret.empty())
    {
        throw domain::DomainError("Cloudinary configuration is incomplete");
    }

    if (options_.cloudinaryFolder.empty())
    {
        options_.cloudinaryFolder = "nova/products";
    }
    if (options_.cloudinaryBusinessFolder.empty())
    {
        options_.cloudinaryBusinessFolder = "nova/businesses";
    }
}

StoredFile FileStorageService::storeProductImage(std::int64_t productId,
                                                 const std::string &sourcePath,
                                                 const domain::menu::ImageMimeType &mimeType,
                                                 const std::string &) const
{
    return storeImageInCloudinary(options_.cloudinaryFolder, "product", productId, sourcePath, mimeType);
}

StoredFile FileStorageService::storeBusinessLogo(std::int64_t businessId,
                                                 const std::string &sourcePath,
                                                 const domain::menu::ImageMimeType &mimeType,
                                                 const std::string &) const
{
    return storeImageInCloudinary(options_.cloudinaryBusinessFolder, "business", businessId, sourcePath, mimeType);
}

StoredFile FileStorageService::storeImageInCloudinary(const std::string &folder,
                                                      const std::string &entityPrefix,
                                                      std::int64_t entityId,
                                                      const std::string &sourcePath,
                                                      const domain::menu::ImageMimeType &mimeType) const
{
    if (!fs::exists(sourcePath))
    {
        throw domain::DomainError("Uploaded temporary file was not found");
    }

    const auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                               std::chrono::system_clock::now().time_since_epoch())
                               .count();
    const std::string publicId = folder + "/" + entityPrefix + "_" + std::to_string(entityId) + "_" + drogon::utils::getUuid();
    const std::string signaturePayload = "public_id=" + publicId + "&timestamp=" + std::to_string(timestamp);
    const std::string signature = buildCloudinarySignature(signaturePayload);
    const auto normalizedPath = fs::path(sourcePath).generic_string();

#ifdef _WIN32
    const std::string curlBinary = "curl.exe";
#else
    const std::string curlBinary = "curl";
#endif

    const std::string command = curlBinary +
                                " -sS -X POST " + shellQuote("https://api.cloudinary.com/v1_1/" + options_.cloudinaryCloudName + "/image/upload") +
                                " -F " + shellQuote("file=@" + normalizedPath + ";type=" + mimeType.value()) +
                                " -F " + shellQuote("api_key=" + options_.cloudinaryApiKey) +
                                " -F " + shellQuote("timestamp=" + std::to_string(timestamp)) +
                                " -F " + shellQuote("public_id=" + publicId) +
                                " -F " + shellQuote("signature=" + signature) +
                                " 2>&1";

    const auto result = runCommand(command);
    if (result.exitCode != 0)
    {
        throw domain::DomainError(result.output.empty() ? "Cloudinary upload command failed" : result.output);
    }

    const auto json = parseJsonOrThrow(result.output);
    const auto error = extractCloudinaryError(json);
    if (!error.empty())
    {
        throw domain::DomainError(error);
    }

    const auto storedPublicId = json["public_id"].asString();
    const auto secureUrl = json["secure_url"].asString();
    if (storedPublicId.empty() || secureUrl.empty())
    {
        throw domain::DomainError("Cloudinary did not return the uploaded image metadata");
    }

    return {basenameFromPath(storedPublicId) + "." + mimeType.extension(), secureUrl};
}

void FileStorageService::deleteFile(const std::string &relativePath) const
{
    if (relativePath.empty())
    {
        return;
    }
    deleteCloudinaryFile(relativePath);
}

void FileStorageService::deleteCloudinaryFile(const std::string &publicIdOrUrl) const
{
    std::string publicId = publicIdOrUrl;
    const std::string uploadPrefix = "https://res.cloudinary.com/" + options_.cloudinaryCloudName + "/image/upload/";
    if (publicId.rfind(uploadPrefix, 0) == 0)
    {
        publicId = publicId.substr(uploadPrefix.size());
        const auto slashPos = publicId.find('/');
        if (slashPos != std::string::npos && publicId.substr(0, slashPos).find("v") == 0)
        {
            publicId = publicId.substr(slashPos + 1);
        }
        const auto dotPos = publicId.find_last_of('.');
        if (dotPos != std::string::npos)
        {
            publicId = publicId.substr(0, dotPos);
        }
    }

    const auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                               std::chrono::system_clock::now().time_since_epoch())
                               .count();
    const std::string signaturePayload = "public_id=" + publicId + "&timestamp=" + std::to_string(timestamp);
    const std::string signature = buildCloudinarySignature(signaturePayload);

#ifdef _WIN32
    const std::string curlBinary = "curl.exe";
#else
    const std::string curlBinary = "curl";
#endif

    const std::string command = curlBinary +
                                " -sS -X POST " + shellQuote("https://api.cloudinary.com/v1_1/" + options_.cloudinaryCloudName + "/image/destroy") +
                                " -F " + shellQuote("api_key=" + options_.cloudinaryApiKey) +
                                " -F " + shellQuote("timestamp=" + std::to_string(timestamp)) +
                                " -F " + shellQuote("public_id=" + publicId) +
                                " -F " + shellQuote("signature=" + signature) +
                                " 2>&1";

    const auto result = runCommand(command);
    if (result.exitCode != 0)
    {
        throw domain::DomainError(result.output.empty() ? "Cloudinary delete command failed" : result.output);
    }

    const auto json = parseJsonOrThrow(result.output);
    const auto error = extractCloudinaryError(json);
    if (!error.empty())
    {
        throw domain::DomainError(error);
    }
}

std::string FileStorageService::resolvePublicFile(const std::string &) const
{
    throw domain::DomainError("Product images are served directly from Cloudinary URLs");
}

std::string FileStorageService::publicUrl(const std::string &storedPath, const std::string &mimeType) const
{
    if (storedPath.empty())
    {
        return {};
    }
    if (storedPath.rfind("http://", 0) == 0 || storedPath.rfind("https://", 0) == 0)
    {
        return storedPath;
    }

    const auto extension = "." + domain::menu::ImageMimeType(mimeType).extension();
    if (storedPath.size() >= extension.size() && storedPath.substr(storedPath.size() - extension.size()) == extension)
    {
        return "https://res.cloudinary.com/" + options_.cloudinaryCloudName + "/image/upload/" + storedPath;
    }

    return "https://res.cloudinary.com/" + options_.cloudinaryCloudName + "/image/upload/" + storedPath + extension;
}

std::string FileStorageService::buildCloudinarySignature(const std::string &payload) const
{
    return hexSha1(payload + options_.cloudinaryApiSecret);
}
}  // namespace starcafe::infrastructure::storage
