#pragma once

#include "application/common/dtos.h"
#include "domain/repositories.h"
#include "infrastructure/security/jwt_service.h"
#include "infrastructure/security/password_hasher.h"

namespace starcafe::application::identity
{
class RegisterUserUseCase
{
  public:
    RegisterUserUseCase(domain::IUserRepository &userRepository,
                        infrastructure::security::PasswordHasher &passwordHasher);
    domain::identity::User execute(const RegisterUserCommand &command);

  private:
    domain::IUserRepository &userRepository_;
    infrastructure::security::PasswordHasher &passwordHasher_;
};

class LoginUseCase
{
  public:
    LoginUseCase(domain::IUserRepository &userRepository,
                 infrastructure::security::PasswordHasher &passwordHasher,
                 infrastructure::security::JwtService &jwtService);
    AuthPayload execute(const LoginCommand &command);

  private:
    domain::IUserRepository &userRepository_;
    infrastructure::security::PasswordHasher &passwordHasher_;
    infrastructure::security::JwtService &jwtService_;
};

class GetCurrentUserUseCase
{
  public:
    explicit GetCurrentUserUseCase(domain::IUserRepository &userRepository);
    domain::identity::User execute(std::int64_t userId);

  private:
    domain::IUserRepository &userRepository_;
};
}  // namespace starcafe::application::identity
