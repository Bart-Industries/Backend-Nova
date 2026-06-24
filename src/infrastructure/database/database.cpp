#include "infrastructure/database/database.h"

#include <drogon/drogon.h>

namespace starcafe::infrastructure::database
{
drogon::orm::DbClientPtr createDbClient(const std::string &databaseUrl)
{
    return drogon::orm::DbClient::newPgClient(databaseUrl, 1);
}
}  // namespace starcafe::infrastructure::database
