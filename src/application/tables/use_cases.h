#pragma once

#include "application/common/dtos.h"
#include "domain/repositories.h"
#include "infrastructure/qr/qr_token_service.h"

namespace starcafe::application::tables
{
class CreateTableUseCase
{
  public:
    CreateTableUseCase(domain::IRestaurantTableRepository &repository, infrastructure::qr::QrTokenService &qrTokenService);
    domain::tables::RestaurantTable execute(const CreateTableCommand &command);

  private:
    domain::IRestaurantTableRepository &repository_;
    infrastructure::qr::QrTokenService &qrTokenService_;
};

class GenerateQrTokenUseCase
{
  public:
    GenerateQrTokenUseCase(domain::IRestaurantTableRepository &repository, infrastructure::qr::QrTokenService &qrTokenService);
    domain::tables::RestaurantTable execute(std::int64_t tableId);

  private:
    domain::IRestaurantTableRepository &repository_;
    infrastructure::qr::QrTokenService &qrTokenService_;
};

class GetTableByQrTokenUseCase
{
  public:
    explicit GetTableByQrTokenUseCase(domain::IRestaurantTableRepository &repository);
    domain::tables::RestaurantTable execute(const std::string &qrToken);

  private:
    domain::IRestaurantTableRepository &repository_;
};

class ListTablesUseCase
{
  public:
    explicit ListTablesUseCase(domain::IRestaurantTableRepository &repository);
    std::vector<domain::tables::RestaurantTable> execute();

  private:
    domain::IRestaurantTableRepository &repository_;
};

class DeactivateTableUseCase
{
  public:
    explicit DeactivateTableUseCase(domain::IRestaurantTableRepository &repository);
    void execute(std::int64_t id);

  private:
    domain::IRestaurantTableRepository &repository_;
};
}  // namespace starcafe::application::tables
