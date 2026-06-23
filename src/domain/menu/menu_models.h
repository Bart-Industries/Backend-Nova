#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace starcafe::domain::menu
{
class ImageMimeType
{
  public:
    explicit ImageMimeType(std::string value);
    const std::string &value() const;
    std::string extension() const;
    static bool isAllowed(const std::string &value);

  private:
    std::string value_;
};

inline ImageMimeType::ImageMimeType(std::string value) : value_(std::move(value))
{
    if (!isAllowed(value_))
    {
        throw domain::DomainError("Solo se permiten imagenes PNG, JPG, JPEG o WEBP");
    }
}

inline const std::string &ImageMimeType::value() const { return value_; }

inline std::string ImageMimeType::extension() const
{
    if (value_ == "image/png")
        return "png";
    if (value_ == "image/webp")
        return "webp";
    return "jpg";
}

inline bool ImageMimeType::isAllowed(const std::string &value)
{
    return value == "image/png" || value == "image/jpeg" || value == "image/jpg" || value == "image/webp";
}

inline FileSize::FileSize(std::int64_t bytes) : bytes_(bytes)
{
    if (bytes_ <= 0)
    {
        throw domain::DomainError("Image file size must be greater than zero");
    }
}

inline std::int64_t FileSize::bytes() const { return bytes_; }

inline void FileSize::validateMax(std::int64_t maxBytes) const
{
    if (bytes_ > maxBytes)
    {
        throw domain::DomainError("Image exceeds the configured maximum size");
    }
}

class FileSize
{
  public:
    explicit FileSize(std::int64_t bytes);
    std::int64_t bytes() const;
    void validateMax(std::int64_t maxBytes) const;

  private:
    std::int64_t bytes_;
};

struct ProductImage
{
    std::int64_t id{};
    std::int64_t productId{};
    std::string fileName;
    std::string filePath;
    std::string mimeType;
    std::int64_t fileSize{};
    bool isMain{true};
    std::string createdAt;
};

struct Category
{
    std::int64_t id{};
    std::string name;
    std::string description;
    bool isActive{true};
};

struct Addon
{
    std::int64_t id{};
    std::string name;
    double price{};
    bool isActive{true};
};

struct Product
{
    std::int64_t id{};
    std::int64_t categoryId{};
    std::string name;
    std::string description;
    double price{};
    bool isAvailable{true};
    bool isActive{true};
    std::vector<Addon> addons;
    std::optional<ProductImage> image;
};
}  // namespace starcafe::domain::menu
#include "domain/common/errors.h"
