#include "infrastructure/security/password_hasher.h"

#include <openssl/evp.h>
#include <openssl/rand.h>

#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace starcafe::infrastructure::security
{
namespace
{
std::string toHex(const std::vector<unsigned char> &input)
{
    std::ostringstream oss;
    for (const auto byte : input)
    {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    return oss.str();
}

std::vector<unsigned char> fromHex(const std::string &value)
{
    std::vector<unsigned char> output;
    output.reserve(value.size() / 2);
    for (std::size_t i = 0; i < value.size(); i += 2)
    {
        output.push_back(static_cast<unsigned char>(std::stoi(value.substr(i, 2), nullptr, 16)));
    }
    return output;
}
}  // namespace

PasswordHasher::PasswordHasher(int cost) : cost_(cost) {}

std::string PasswordHasher::hash(const std::string &plainText) const
{
    std::vector<unsigned char> salt(16);
    if (RAND_bytes(salt.data(), static_cast<int>(salt.size())) != 1)
    {
        throw std::runtime_error("Unable to generate password salt");
    }

    const int iterations = 1 << cost_;
    std::vector<unsigned char> digest(32);
    if (PKCS5_PBKDF2_HMAC(plainText.c_str(),
                          static_cast<int>(plainText.size()),
                          salt.data(),
                          static_cast<int>(salt.size()),
                          iterations,
                          EVP_sha256(),
                          static_cast<int>(digest.size()),
                          digest.data()) != 1)
    {
        throw std::runtime_error("Unable to hash password");
    }

    return std::to_string(cost_) + ":" + toHex(salt) + ":" + toHex(digest);
}

bool PasswordHasher::verify(const std::string &plainText, const std::string &encoded) const
{
    const auto first = encoded.find(':');
    const auto second = encoded.find(':', first + 1);
    if (first == std::string::npos || second == std::string::npos)
    {
        return false;
    }

    const auto storedCost = std::stoi(encoded.substr(0, first));
    const auto salt = fromHex(encoded.substr(first + 1, second - first - 1));
    const auto expected = fromHex(encoded.substr(second + 1));

    std::vector<unsigned char> digest(expected.size());
    if (PKCS5_PBKDF2_HMAC(plainText.c_str(),
                          static_cast<int>(plainText.size()),
                          salt.data(),
                          static_cast<int>(salt.size()),
                          1 << storedCost,
                          EVP_sha256(),
                          static_cast<int>(digest.size()),
                          digest.data()) != 1)
    {
        return false;
    }

    return digest == expected;
}
}  // namespace starcafe::infrastructure::security
