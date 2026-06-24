#include "infrastructure/storage/file_storage_service.h"

#include "domain/common/errors.h"

#include <drogon/utils/Utilities.h>

#include <filesystem>

namespace starcafe::infrastructure::storage
{
namespace fs = std::filesystem;

FileStorageService::FileStorageService(std::string uploadDir) : uploadDir_(std::move(uploadDir))
{
    fs::create_directories(uploadDir_);
}

StoredFile FileStorageService::storeProductImage(std::int64_t productId,
                                                 const std::string &sourcePath,
                                                 const domain::menu::ImageMimeType &mimeType,
                                                 const std::string &) const
{
    if (!fs::exists(sourcePath))
    {
        throw domain::DomainError("Uploaded temporary file was not found");
    }

    const std::string fileName = "product_" + std::to_string(productId) + "_" + drogon::utils::getUuid() + "." + mimeType.extension();
    const fs::path destination = fs::path(uploadDir_) / fileName;
    fs::copy_file(sourcePath, destination, fs::copy_options::overwrite_existing);
    return {fileName, fileName};
}

void FileStorageService::deleteFile(const std::string &relativePath) const
{
    const fs::path base = fs::weakly_canonical(fs::path(uploadDir_));
    const fs::path file = fs::weakly_canonical(base / relativePath);
    if (file.string().rfind(base.string(), 0) != 0)
    {
        throw domain::DomainError("Invalid file path");
    }
    if (fs::exists(file))
    {
        fs::remove(file);
    }
}

std::string FileStorageService::resolvePublicFile(const std::string &fileName) const
{
    const fs::path base = fs::weakly_canonical(fs::path(uploadDir_));
    const fs::path file = fs::weakly_canonical(base / fileName);
    if (file.string().rfind(base.string(), 0) != 0)
    {
        throw domain::DomainError("Invalid file path");
    }
    return file.string();
}

const std::string &FileStorageService::uploadDir() const { return uploadDir_; }
}  // namespace starcafe::infrastructure::storage
