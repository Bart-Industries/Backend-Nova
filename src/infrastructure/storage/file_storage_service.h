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

class FileStorageService
{
  public:
    explicit FileStorageService(std::string uploadDir);
    StoredFile storeProductImage(std::int64_t productId,
                                 const std::string &sourcePath,
                                 const domain::menu::ImageMimeType &mimeType,
                                 const std::string &originalFileName) const;
    void deleteFile(const std::string &relativePath) const;
    std::string resolvePublicFile(const std::string &fileName) const;
    const std::string &uploadDir() const;

  private:
    std::string uploadDir_;
};
}  // namespace starcafe::infrastructure::storage
