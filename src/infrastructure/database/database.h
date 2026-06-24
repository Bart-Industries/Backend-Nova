#pragma once

#include <drogon/orm/DbClient.h>

namespace starcafe::infrastructure::database
{
drogon::orm::DbClientPtr createDbClient(const std::string &databaseUrl);
}
