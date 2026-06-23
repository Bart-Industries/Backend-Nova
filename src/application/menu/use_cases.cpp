#include "application/menu/use_cases.h"
#include "domain/common/errors.h"
#include "infrastructure/storage/file_storage_service.h"

namespace starcafe::application::menu
{
CreateCategoryUseCase::CreateCategoryUseCase(domain::ICategoryRepository &repository) : repository_(repository) {}
domain::menu::Category CreateCategoryUseCase::execute(const CreateCategoryCommand &command)
{
    if (command.name.empty())
    {
        throw domain::DomainError("Category name is required");
    }
    return repository_.create({0, command.name, command.description, true});
}

ListCategoriesUseCase::ListCategoriesUseCase(domain::ICategoryRepository &repository) : repository_(repository) {}
std::vector<domain::menu::Category> ListCategoriesUseCase::execute(bool onlyActive) { return repository_.listAll(onlyActive); }

CreateProductUseCase::CreateProductUseCase(domain::IProductRepository &repository) : repository_(repository) {}
domain::menu::Product CreateProductUseCase::execute(const CreateProductCommand &command)
{
    if (command.name.empty() || command.price <= 0)
    {
        throw domain::DomainError("Product name and price are required");
    }
    return repository_.create({0, command.categoryId, command.name, command.description, command.price, true, true, {}});
}

UpdateProductUseCase::UpdateProductUseCase(domain::IProductRepository &repository) : repository_(repository) {}
domain::menu::Product UpdateProductUseCase::execute(std::int64_t id, const UpdateProductCommand &command)
{
    if (command.name.empty() || command.price <= 0)
    {
        throw domain::DomainError("Product name and price are required");
    }
    return repository_.update(id,
                              {0,
                               command.categoryId,
                               command.name,
                               command.description,
                               command.price,
                               command.isAvailable,
                               true,
                               {}});
}

MarkProductUnavailableUseCase::MarkProductUnavailableUseCase(domain::IProductRepository &repository) : repository_(repository) {}
void MarkProductUnavailableUseCase::execute(std::int64_t id) { repository_.markUnavailable(id); }

DeactivateProductUseCase::DeactivateProductUseCase(domain::IProductRepository &repository) : repository_(repository) {}
void DeactivateProductUseCase::execute(std::int64_t id) { repository_.deactivate(id); }

CreateAddonUseCase::CreateAddonUseCase(domain::IAddonRepository &repository) : repository_(repository) {}
domain::menu::Addon CreateAddonUseCase::execute(const CreateAddonCommand &command)
{
    if (command.name.empty() || command.price < 0)
    {
        throw domain::DomainError("Addon name and price are required");
    }
    return repository_.create({0, command.name, command.price, true});
}

AssignAddonToProductUseCase::AssignAddonToProductUseCase(domain::IProductRepository &repository) : repository_(repository) {}
void AssignAddonToProductUseCase::execute(std::int64_t productId, std::int64_t addonId) { repository_.assignAddon(productId, addonId); }

GetPublicMenuUseCase::GetPublicMenuUseCase(domain::IProductRepository &repository) : repository_(repository) {}
std::vector<domain::menu::Product> GetPublicMenuUseCase::execute() { return repository_.listPublicMenu(); }

ListProductsUseCase::ListProductsUseCase(domain::IProductRepository &repository) : repository_(repository) {}
std::vector<domain::menu::Product> ListProductsUseCase::execute(bool onlyActive) { return repository_.listAll(onlyActive); }

UploadProductImageUseCase::UploadProductImageUseCase(domain::IProductRepository &productRepository,
                                                     domain::IProductImageRepository &imageRepository,
                                                     infrastructure::storage::FileStorageService &fileStorageService,
                                                     std::int64_t maxFileBytes)
    : productRepository_(productRepository),
      imageRepository_(imageRepository),
      fileStorageService_(fileStorageService),
      maxFileBytes_(maxFileBytes)
{
}

domain::menu::ProductImage UploadProductImageUseCase::execute(const UploadProductImageCommand &command)
{
    const auto product = productRepository_.findById(command.productId);
    if (!product.has_value())
    {
        throw domain::DomainError("Product not found");
    }

    domain::menu::ImageMimeType mimeType(command.mimeType);
    domain::menu::FileSize fileSize(command.fileSize);
    fileSize.validateMax(maxFileBytes_);

    std::optional<domain::menu::ProductImage> previousImage = imageRepository_.findMainByProductId(command.productId);
    const auto storedFile = fileStorageService_.storeProductImage(command.productId, command.tempFilePath, mimeType, command.originalFileName);

    domain::menu::ProductImage image;
    image.productId = command.productId;
    image.fileName = storedFile.fileName;
    image.filePath = storedFile.relativePath;
    image.mimeType = mimeType.value();
    image.fileSize = fileSize.bytes();
    image.isMain = true;

    auto saved = imageRepository_.upsertMain(image);
    if (command.deleteOldPhysicalFile && previousImage.has_value() && previousImage->filePath != saved.filePath)
    {
        fileStorageService_.deleteFile(previousImage->filePath);
    }
    return saved;
}

ReplaceProductImageUseCase::ReplaceProductImageUseCase(UploadProductImageUseCase &uploadUseCase) : uploadUseCase_(uploadUseCase) {}
domain::menu::ProductImage ReplaceProductImageUseCase::execute(const UploadProductImageCommand &command)
{
    auto replaceCommand = command;
    replaceCommand.deleteOldPhysicalFile = true;
    return uploadUseCase_.execute(replaceCommand);
}

DeleteProductImageUseCase::DeleteProductImageUseCase(domain::IProductRepository &productRepository,
                                                     domain::IProductImageRepository &imageRepository,
                                                     infrastructure::storage::FileStorageService &fileStorageService)
    : productRepository_(productRepository), imageRepository_(imageRepository), fileStorageService_(fileStorageService)
{
}

void DeleteProductImageUseCase::execute(std::int64_t productId, bool deletePhysicalFile)
{
    const auto product = productRepository_.findById(productId);
    if (!product.has_value())
    {
        throw domain::DomainError("Product not found");
    }

    const auto existing = imageRepository_.findMainByProductId(productId);
    if (!existing.has_value())
    {
        throw domain::DomainError("Product does not have a main image");
    }

    imageRepository_.deleteMain(productId);
    if (deletePhysicalFile)
    {
        fileStorageService_.deleteFile(existing->filePath);
    }
}
}  // namespace starcafe::application::menu
