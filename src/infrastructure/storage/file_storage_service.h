#pragma once

#include "domain/menu/menu_models.h"

#include <string>

namespace starcafe::infrastructure::storage
{
struct StoredFile
{
    std::string fileName;
    std::string relativePath;
};

struct FileStorageOptions
{
    std::string cloudinaryCloudName;
    std::string cloudinaryApiKey;
    std::string cloudinaryApiSecret;
    std::string cloudinaryFolder{"nova/products"};
    std::string cloudinaryBusinessFolder{"nova/businesses"};
};

class FileStorageService
{
  public:
    explicit FileStorageService(FileStorageOptions options);
    StoredFile storeProductImage(std::int64_t productId,
                                 const std::string &sourcePath,
                                 const domain::menu::ImageMimeType &mimeType,
                                 const std::string &originalFileName) const;
    StoredFile storeBusinessLogo(std::int64_t businessId,
                                 const std::string &sourcePath,
                                 const domain::menu::ImageMimeType &mimeType,
                                 const std::string &originalFileName) const;
    void deleteFile(const std::string &relativePath) const;
    std::string publicUrl(const std::string &storedPath, const std::string &mimeType) const;

  private:
    std::string buildCloudinarySignature(const std::string &payload) const;
    StoredFile storeImageInCloudinary(const std::string &folder,
                                      const std::string &entityPrefix,
                                      std::int64_t entityId,
                                      const std::string &sourcePath,
                                      const domain::menu::ImageMimeType &mimeType) const;
    void deleteCloudinaryFile(const std::string &publicId) const;

    FileStorageOptions options_;
};
}  // namespace starcafe::infrastructure::storage
