#include "application/identity/use_cases.h"
#include "domain/common/errors.h"

namespace starcafe::application::identity
{
RegisterUserUseCase::RegisterUserUseCase(domain::IUserRepository &userRepository,
                                         infrastructure::security::PasswordHasher &passwordHasher)
    : userRepository_(userRepository), passwordHasher_(passwordHasher)
{
}

domain::identity::User RegisterUserUseCase::execute(const RegisterUserCommand &command)
{
    if (command.name.empty() || command.email.empty() || command.password.empty())
    {
        throw domain::DomainError("Name, email and password are required");
    }

    if (userRepository_.findByEmail(command.email).has_value())
    {
        throw domain::DomainError("Email already registered");
    }

    if (command.role == domain::UserRole::ADMIN && userRepository_.countActiveAdmins() >= 3)
    {
        throw domain::DomainError("Maximum of 3 active admin users reached");
    }

    domain::identity::User user;
    user.name = command.name;
    user.email = command.email;
    user.passwordHash = passwordHasher_.hash(command.password);
    user.role = command.role;
    user.isActive = true;
    return userRepository_.create(user);
}

LoginUseCase::LoginUseCase(domain::IUserRepository &userRepository,
                           infrastructure::security::PasswordHasher &passwordHasher,
                           infrastructure::security::JwtService &jwtService)
    : userRepository_(userRepository), passwordHasher_(passwordHasher), jwtService_(jwtService)
{
}

AuthPayload LoginUseCase::execute(const LoginCommand &command)
{
    const auto user = userRepository_.findByEmail(command.email);
    if (!user.has_value() || !user->isActive || !passwordHasher_.verify(command.password, user->passwordHash))
    {
        throw domain::DomainError("Invalid credentials");
    }

    AuthPayload payload;
    payload.token = jwtService_.createToken(user->id, user->name, user->email, user->role);
    payload.userId = user->id;
    payload.role = domain::toString(user->role);
    payload.name = user->name;
    payload.email = user->email;
    return payload;
}

GetCurrentUserUseCase::GetCurrentUserUseCase(domain::IUserRepository &userRepository)
    : userRepository_(userRepository)
{
}

domain::identity::User GetCurrentUserUseCase::execute(std::int64_t userId)
{
    const auto user = userRepository_.findById(userId);
    if (!user.has_value() || !user->isActive)
    {
        throw domain::DomainError("User not found");
    }
    return *user;
}
}  // namespace starcafe::application::identity
