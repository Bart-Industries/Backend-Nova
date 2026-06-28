#pragma once

#include "application/common/dtos.h"
#include "domain/repositories.h"

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
}  // namespace starcafe::application::businesses
