cmake_minimum_required(VERSION 3.20)
project(NovaBackend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Drogon CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

add_executable(nova
    src/main.cpp
    src/application/businesses/use_cases.cpp
    src/application/identity/use_cases.cpp
    src/application/menu/use_cases.cpp
    src/application/tables/use_cases.cpp
    src/application/orders/use_cases.cpp
    src/application/payments/use_cases.cpp
    src/infrastructure/config/env_loader.cpp
    src/infrastructure/config/app_config.cpp
    src/infrastructure/database/database.cpp
    src/infrastructure/repositories/postgres_repositories.cpp
    src/infrastructure/storage/file_storage_service.cpp
    src/infrastructure/security/password_hasher.cpp
    src/infrastructure/security/jwt_service.cpp
    src/infrastructure/security/auth_filters.cpp
    src/infrastructure/qr/qr_token_service.cpp
    src/interfaces/rest/api_controller.cpp
    src/interfaces/rest/business_branding_controller.cpp
    src/interfaces/rest/docs_controller.cpp
    src/interfaces/rest/product_image_controller.cpp
)

target_include_directories(nova PRIVATE src)
target_link_libraries(nova PRIVATE Drogon::Drogon OpenSSL::Crypto)

if(MSVC)
    target_compile_options(nova PRIVATE /W4 /permissive-)
else()
    target_compile_options(nova PRIVATE -Wall -Wextra -Wpedantic)
endif()
