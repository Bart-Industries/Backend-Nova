#include "application/businesses/use_cases.h"
#include "domain/common/errors.h"
#include "domain/businesses/theme_catalog.h"
#include "domain/menu/menu_models.h"
#include "infrastructure/storage/file_storage_service.h"

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

GetBusinessUseCase::GetBusinessUseCase(domain::IBusinessRepository &repository) : repository_(repository) {}
domain::businesses::Business GetBusinessUseCase::execute(std::int64_t businessId)
{
    const auto business = repository_.findById(businessId);
    if (!business.has_value())
    {
        throw domain::DomainError("Business not found");
    }
    return *business;
}

UpdateBusinessThemeUseCase::UpdateBusinessThemeUseCase(domain::IBusinessRepository &repository) : repository_(repository) {}
domain::businesses::Business UpdateBusinessThemeUseCase::execute(std::int64_t businessId, const UpdateBusinessThemeCommand &command)
{
    if (command.themeKey.empty())
    {
        throw domain::DomainError("themeKey is required");
    }

    const auto business = repository_.findById(businessId);
    if (!business.has_value())
    {
        throw domain::DomainError("Business not found");
    }
    if (!business->isActive)
    {
        throw domain::DomainError("Inactive businesses cannot be customized");
    }

    const auto &theme = domain::businesses::themeByKey(command.themeKey);
    return repository_.updateTheme(businessId, std::string(theme.primaryColor));
}

UpdateBusinessLogoUseCase::UpdateBusinessLogoUseCase(domain::IBusinessRepository &repository,
                                                     infrastructure::storage::FileStorageService &fileStorageService,
                                                     std::int64_t maxFileBytes)
    : repository_(repository), fileStorageService_(fileStorageService), maxFileBytes_(maxFileBytes)
{
}

domain::businesses::Business UpdateBusinessLogoUseCase::execute(std::int64_t businessId, const UploadBusinessLogoCommand &command)
{
    const auto business = repository_.findById(businessId);
    if (!business.has_value())
    {
        throw domain::DomainError("Business not found");
    }
    if (!business->isActive)
    {
        throw domain::DomainError("Inactive businesses cannot be customized");
    }

    domain::menu::ImageMimeType mimeType(command.mimeType);
    domain::menu::FileSize fileSize(command.fileSize);
    fileSize.validateMax(maxFileBytes_);

    const auto storedFile = fileStorageService_.storeBusinessLogo(businessId, command.tempFilePath, mimeType, command.originalFileName);
    const auto updated = repository_.updateLogo(businessId, storedFile.relativePath);
    if (!business->logoUrl.empty() && business->logoUrl != updated.logoUrl)
    {
        fileStorageService_.deleteFile(business->logoUrl);
    }
    return updated;
}
}  // namespace starcafe::application::businesses
