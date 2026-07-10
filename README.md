# StarCafe Backend

Backend monolitico modular en C++ moderno con Drogon, PostgreSQL y una estructura inspirada en DDD + Clean Architecture.

## Requisitos

- CMake 3.20+
- Compilador con soporte C++20
- Drogon
- OpenSSL
- PostgreSQL accesible desde `DATABASE_URL`

## Variables de entorno

1. Copia `.env.example` a `.env`.
2. Completa:
   - `APP_PORT`
   - `DATABASE_URL`
   - `JWT_SECRET`
   - `JWT_EXPIRES_IN`
   - `CORS_ALLOWED_ORIGINS`
   - `FRONTEND_BASE_URL`
   - `BCRYPT_COST`
   - `APP_THREADS`
   - `CLOUDINARY_CLOUD_NAME`
   - `CLOUDINARY_API_KEY`
   - `CLOUDINARY_API_SECRET`
   - `CLOUDINARY_FOLDER`
   - `CLOUDINARY_BUSINESS_FOLDER`
   - `MAX_PRODUCT_IMAGE_SIZE_MB`

## Ejecutar localmente

```powershell
cmake -S . -B build
cmake --build build
.\build\Debug\nova.exe
```

En Linux/macOS:

```bash
cmake -S . -B build
cmake --build build
./build/nova
```

## Docker local

Construir la imagen:

```bash
docker build -t nova-backend .
```

Ejecutar el contenedor:

```bash
docker run --rm -p 8080:8080 --env-file .env nova-backend
```

## Deploy en Render

Este proyecto debe desplegarse en Render como servicio `Docker`, no como runtime nativo.

### Pasos

1. Crea un nuevo `Web Service`.
2. Conecta tu repositorio.
3. En `Language`, elige `Docker`.
4. Render detectara automaticamente el `Dockerfile`.
5. Configura las variables de entorno del servicio.
6. Usa como `Health Check Path`: `/openapi.json`

### Variables de entorno recomendadas en Render

- `DATABASE_URL`
- `JWT_SECRET`
- `JWT_EXPIRES_IN`
- `CORS_ALLOWED_ORIGINS`
- `FRONTEND_BASE_URL`
- `BCRYPT_COST`
- `APP_THREADS`
- `CLOUDINARY_CLOUD_NAME`
- `CLOUDINARY_API_KEY`
- `CLOUDINARY_API_SECRET`
- `CLOUDINARY_FOLDER`
- `CLOUDINARY_BUSINESS_FOLDER`
- `MAX_PRODUCT_IMAGE_SIZE_MB`

Notas:

- No necesitas definir `APP_PORT` en Render si usas Docker; el backend ahora acepta `PORT` automaticamente.
- Render inyecta `PORT` por defecto en contenedores.
- Si tu frontend vive en otro dominio, ajusta `CORS_ALLOWED_ORIGINS` y `FRONTEND_BASE_URL`.

## Endpoints principales

Documentacion interactiva:

- `GET /docs`
- `GET /openapi.json`

### Auth

- `POST /api/v1/auth/register`
- `POST /api/v1/auth/login`
- `GET /api/v1/auth/me`

### Publico mesa

- `GET /api/v1/tables/qr/{qrToken}`
- `GET /api/v1/public/menu`
- `POST /api/v1/public/tables/{qrToken}/orders`
- `GET /api/v1/public/orders/{orderId}/status?qrToken={qrToken}`

### Kitchen

- `GET /api/v1/kitchen/orders`
- `PATCH /api/v1/kitchen/orders/{orderId}/preparing`
- `PATCH /api/v1/kitchen/order-items/{itemId}/ready`
- `PATCH /api/v1/kitchen/orders/{orderId}/ready`
- `GET /api/v1/kitchen/orders/history`

### Admin y caja

- `GET /api/v1/admin/users`
- `POST /api/v1/admin/users`
- `PATCH /api/v1/admin/users/{id}/deactivate`
- `GET /api/v1/admin/tables`
- `POST /api/v1/admin/tables`
- `PATCH /api/v1/admin/tables/{id}/deactivate`
- `GET /api/v1/admin/categories`
- `POST /api/v1/admin/categories`
- `PATCH /api/v1/admin/categories/{id}`
- `GET /api/v1/admin/products`
- `POST /api/v1/admin/products`
- `PATCH /api/v1/admin/products/{id}`
- `PATCH /api/v1/admin/products/{id}/unavailable`
- `PATCH /api/v1/admin/products/{id}/deactivate`
- `POST /api/v1/admin/products/{productId}/image`
- `PATCH /api/v1/admin/products/{productId}/image`
- `DELETE /api/v1/admin/products/{productId}/image`
- `GET /api/v1/admin/addons`
- `POST /api/v1/admin/addons`
- `PATCH /api/v1/admin/addons/{id}`
- `POST /api/v1/admin/products/{productId}/addons/{addonId}`
- `GET /api/v1/admin/orders`
- `GET /api/v1/admin/orders/history`
- `PATCH /api/v1/admin/orders/{orderId}/cancel`
- `GET /api/v1/admin/cashier/orders/search`
- `POST /api/v1/admin/cashier/orders/{orderId}/pay`

## Ejemplos para Postman / Thunder Client

### Registrar usuario admin

```json
{
  "name": "Admin Principal",
  "email": "admin@starcafe.com",
  "password": "StrongPassword123!",
  "role": "ADMIN"
}
```

### Login

```json
{
  "email": "admin@starcafe.com",
  "password": "StrongPassword123!"
}
```

### Crear pedido publico

```json
{
  "customerName": "Lucia",
  "items": [
    {
      "productId": 1,
      "quantity": 2,
      "notes": "Sin cebolla",
      "addonIds": [1, 2]
    }
  ]
}
```

### Pagar pedido

```json
{
  "amount": 49.8
}
```

### Subir imagen principal de producto

Usa `multipart/form-data` con el campo `image`.

```bash
curl -X POST http://localhost:8080/api/v1/admin/products/1/image \
  -H "Authorization: Bearer TOKEN" \
  -F "image=@./capuccino.webp"
```

## Notas

- `POST /api/v1/auth/register` solo queda publico para el bootstrap inicial del primer `SUPER_ADMIN`; despues exige autenticacion `SUPER_ADMIN`.
- El total del pedido siempre se recalcula en backend.
- El estado publico de un pedido ahora exige `qrToken` de la mesa duena del pedido para evitar consultas cruzadas por `orderId`.
- Las imagenes de productos y logos de negocio se guardan en Cloudinary; PostgreSQL solo almacena metadata y URLs.
- Los queries SQL asumen nombres de columnas convencionales sobre la base dada. Si tu esquema usa variantes como `restaurant_table_id` en lugar de `table_id`, ajusta los repositorios sin cambiar las reglas de dominio.
- Productos, categorias, adicionales, mesas y usuarios se desactivan con `is_active = false`.
