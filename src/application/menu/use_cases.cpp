#include "application/menu/use_cases.h"
#include "domain/common/errors.h"

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
}  // namespace starcafe::application::menu
