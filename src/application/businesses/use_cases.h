#pragma once

#include "application/common/dtos.h"
#include "domain/repositories.h"

namespace starcafe::infrastructure::storage
{
class FileStorageService;
}

namespace starcafe::application::businesses
{
class CreateBusinessUseCase
{
  public:
    explicit CreateBusinessUseCase(domain::IBusinessRepository &repository);
    domain::businesses::Business execute(const CreateBusinessCommand &command);

  private:
    domain::IBusinessRepository &repository_;
};

class ListBusinessesUseCase
{
  public:
    explicit ListBusinessesUseCase(domain::IBusinessRepository &repository);
    std::vector<domain::businesses::Business> execute(bool onlyActive = false);

  private:
    domain::IBusinessRepository &repository_;
};

class GetBusinessUseCase
{
  public:
    explicit GetBusinessUseCase(domain::IBusinessRepository &repository);
    domain::businesses::Business execute(std::int64_t businessId);

  private:
    domain::IBusinessRepository &repository_;
};

class UpdateBusinessThemeUseCase
{
  public:
    explicit UpdateBusinessThemeUseCase(domain::IBusinessRepository &repository);
    domain::businesses::Business execute(std::int64_t businessId, const UpdateBusinessThemeCommand &command);

  private:
    domain::IBusinessRepository &repository_;
};

class UpdateBusinessLogoUseCase
{
  public:
    UpdateBusinessLogoUseCase(domain::IBusinessRepository &repository,
                              infrastructure::storage::FileStorageService &fileStorageService,
                              std::int64_t maxFileBytes);
    domain::businesses::Business execute(std::int64_t businessId, const UploadBusinessLogoCommand &command);

  private:
    domain::IBusinessRepository &repository_;
    infrastructure::storage::FileStorageService &fileStorageService_;
    std::int64_t maxFileBytes_;
};
}  // namespace starcafe::application::businesses
