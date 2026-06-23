#include "application/tables/use_cases.h"
#include "domain/common/errors.h"

namespace starcafe::application::tables
{
CreateTableUseCase::CreateTableUseCase(domain::IRestaurantTableRepository &repository,
                                       infrastructure::qr::QrTokenService &qrTokenService)
    : repository_(repository), qrTokenService_(qrTokenService)
{
}

domain::tables::RestaurantTable CreateTableUseCase::execute(const CreateTableCommand &command)
{
    if (command.tableNumber <= 0)
    {
        throw domain::DomainError("Table number must be greater than zero");
    }
    return repository_.create({0, command.tableNumber, qrTokenService_.generateToken(), true});
}

GenerateQrTokenUseCase::GenerateQrTokenUseCase(domain::IRestaurantTableRepository &repository,
                                               infrastructure::qr::QrTokenService &qrTokenService)
    : repository_(repository), qrTokenService_(qrTokenService)
{
}

domain::tables::RestaurantTable GenerateQrTokenUseCase::execute(std::int64_t tableId)
{
    return repository_.updateQrToken(tableId, qrTokenService_.generateToken());
}

GetTableByQrTokenUseCase::GetTableByQrTokenUseCase(domain::IRestaurantTableRepository &repository) : repository_(repository) {}
domain::tables::RestaurantTable GetTableByQrTokenUseCase::execute(const std::string &qrToken)
{
    const auto table = repository_.findByQrToken(qrToken);
    if (!table.has_value() || !table->isActive)
    {
        throw domain::DomainError("Table not found");
    }
    return *table;
}

ListTablesUseCase::ListTablesUseCase(domain::IRestaurantTableRepository &repository) : repository_(repository) {}
std::vector<domain::tables::RestaurantTable> ListTablesUseCase::execute() { return repository_.listActive(); }

DeactivateTableUseCase::DeactivateTableUseCase(domain::IRestaurantTableRepository &repository) : repository_(repository) {}
void DeactivateTableUseCase::execute(std::int64_t id) { repository_.deactivate(id); }
}  // namespace starcafe::application::tables
