#include "application/businesses/use_cases.h"
#include "domain/common/errors.h"

namespace starcafe::application::businesses
{
CreateBusinessUseCase::CreateBusinessUseCase(domain::IBusinessRepository &repository) : repository_(repository) {}

domain::businesses::Business CreateBusinessUseCase::execute(const CreateBusinessCommand &command)
{
    if (command.name.empty() || command.slug.empty())
    {
        throw domain::DomainError("Business name and slug are required");
    }
    if (repository_.findBySlug(command.slug).has_value())
    {
        throw domain::DomainError("Business slug already exists");
    }

    domain::businesses::Business business;
    business.name = command.name;
    business.slug = command.slug;
    business.logoUrl = command.logoUrl;
    business.primaryColor = command.primaryColor;
    business.isActive = true;
    return repository_.create(business);
}

ListBusinessesUseCase::ListBusinessesUseCase(domain::IBusinessRepository &repository) : repository_(repository) {}
std::vector<domain::businesses::Business> ListBusinessesUseCase::execute(bool onlyActive) { return repository_.listAll(onlyActive); }
}  // namespace starcafe::application::businesses
