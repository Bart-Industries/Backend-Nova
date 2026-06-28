#pragma once

#include "application/common/dtos.h"
#include "domain/repositories.h"

namespace starcafe::application::menu
{
class CreateCategoryUseCase
{
  public:
    explicit CreateCategoryUseCase(domain::ICategoryRepository &repository);
    domain::menu::Category execute(std::int64_t businessId, const CreateCategoryCommand &command);

  private:
    domain::ICategoryRepository &repository_;
};

class ListCategoriesUseCase
{
  public:
    explicit ListCategoriesUseCase(domain::ICategoryRepository &repository);
    std::vector<domain::menu::Category> execute(std::int64_t businessId, bool onlyActive = true);

  private:
    domain::ICategoryRepository &repository_;
};

class CreateProductUseCase
{
  public:
    explicit CreateProductUseCase(domain::IProductRepository &repository);
    domain::menu::Product execute(const CreateProductCommand &command);

  private:
    domain::IProductRepository &repository_;
};

class UpdateProductUseCase
{
  public:
    explicit UpdateProductUseCase(domain::IProductRepository &repository);
    domain::menu::Product execute(std::int64_t id, const UpdateProductCommand &command);

  private:
    domain::IProductRepository &repository_;
};

class MarkProductUnavailableUseCase
{
  public:
    explicit MarkProductUnavailableUseCase(domain::IProductRepository &repository);
    void execute(std::int64_t businessId, std::int64_t id);

  private:
    domain::IProductRepository &repository_;
};

class ActivateProductUseCase
{
  public:
    explicit ActivateProductUseCase(domain::IProductRepository &repository);
    void execute(std::int64_t businessId, std::int64_t id);

  private:
    domain::IProductRepository &repository_;
};

class DeactivateProductUseCase
{
  public:
    explicit DeactivateProductUseCase(domain::IProductRepository &repository);
    void execute(std::int64_t businessId, std::int64_t id);

  private:
    domain::IProductRepository &repository_;
};

class CreateAddonUseCase
{
  public:
    explicit CreateAddonUseCase(domain::IAddonRepository &repository);
    domain::menu::Addon execute(const CreateAddonCommand &command);

  private:
    domain::IAddonRepository &repository_;
};

class AssignAddonToProductUseCase
{
  public:
    AssignAddonToProductUseCase(domain::IProductRepository &productRepository, domain::IAddonRepository &addonRepository);
    void execute(std::int64_t businessId, std::int64_t productId, std::int64_t addonId);

  private:
    domain::IProductRepository &productRepository_;
    domain::IAddonRepository &addonRepository_;
};

class GetPublicMenuUseCase
{
  public:
    GetPublicMenuUseCase(domain::IProductRepository &repository, domain::IBusinessRepository &businessRepository);
    std::vector<domain::menu::Product> execute(const std::string &businessSlug);

  private:
    domain::IProductRepository &repository_;
    domain::IBusinessRepository &businessRepository_;
};

class ListProductsUseCase
{
  public:
    explicit ListProductsUseCase(domain::IProductRepository &repository);
    std::vector<domain::menu::Product> execute(std::int64_t businessId, bool onlyActive = false);

  private:
    domain::IProductRepository &repository_;
};

class UploadProductImageUseCase
{
  public:
    UploadProductImageUseCase(domain::IProductRepository &productRepository,
                              domain::IProductImageRepository &imageRepository,
                              infrastructure::storage::FileStorageService &fileStorageService,
                              std::int64_t maxFileBytes);
    domain::menu::ProductImage execute(std::int64_t businessId, const UploadProductImageCommand &command);

  private:
    domain::IProductRepository &productRepository_;
    domain::IProductImageRepository &imageRepository_;
    infrastructure::storage::FileStorageService &fileStorageService_;
    std::int64_t maxFileBytes_;
};

class ReplaceProductImageUseCase
{
  public:
    explicit ReplaceProductImageUseCase(UploadProductImageUseCase &uploadUseCase);
    domain::menu::ProductImage execute(std::int64_t businessId, const UploadProductImageCommand &command);

  private:
    UploadProductImageUseCase &uploadUseCase_;
};

class DeleteProductImageUseCase
{
  public:
    DeleteProductImageUseCase(domain::IProductRepository &productRepository,
                              domain::IProductImageRepository &imageRepository,
                              infrastructure::storage::FileStorageService &fileStorageService);
    void execute(std::int64_t businessId, std::int64_t productId, bool deletePhysicalFile);

  private:
    domain::IProductRepository &productRepository_;
    domain::IProductImageRepository &imageRepository_;
    infrastructure::storage::FileStorageService &fileStorageService_;
};
}  // namespace starcafe::application::menu
