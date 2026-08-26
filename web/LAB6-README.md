# FridgeFusion Lab #6 - Микросервисная архитектура

## Обзор

Лабораторная работа #6 представляет декомпозицию монолитного бэкенда на **четыре микросервиса** с межсервисным взаимодействием через REST API.

### Архитектура

```
Client → Caddy (Load Balancer) → Gateway Service → Auth Service → Core Service → Data Service → PostgreSQL
```

**Всего контейнеров:** 18
- 12 микросервисов (4 сервиса × 3 инстанса)
- 2 базы данных (master + slave)
- 1 load balancer (Caddy)
- 3 мониторинг (Grafana + Loki + Promtail)

| Слой | Сервисы | Порт | Описание |
|------|---------|------|----------|
| Gateway | gateway-service-1/2/3 | 8080 | Внешний API, маршрутизация запросов |
| Auth | auth-service-1/2/3 | 8083 | JWT-аутентификация, управление токенами |
| Core | core-service-1/2/3 | 8081 | Бизнес-логика, валидация |
| Data | data-service-1/2/3 | 8082 | Доступ к данным, репозитории |

---

## Что было сделано

### 1. Data Service — слой доступа к данным

**Расположение:** [services/data-service/](services/data-service/)

Data Service предоставляет REST API над существующими репозиториями из `libs/infrastructure/`. Он является единственным сервисом, который напрямую работает с PostgreSQL.

**Ключевые файлы:**
- [services/data-service/src/main.cpp](services/data-service/src/main.cpp) — REST API эндпоинты для всех сущностей (users, recipes, ingredients, favorites, reviews)
- [services/data-service/CMakeLists.txt](services/data-service/CMakeLists.txt) — сборка с Qt6 и репозиториями из libs/
- [services/data-service/docker/Dockerfile](services/data-service/docker/Dockerfile) — multi-stage build

**Эндпоинты Data Service:**
```
GET/POST       /api/data/users
GET            /api/data/users/:id
GET            /api/data/users/email/:email
GET/POST       /api/data/recipes
GET            /api/data/recipes/:id/preview
GET            /api/data/recipes/:id/detail
GET/POST       /api/data/categories
GET/POST       /api/data/ingredients
GET/POST       /api/data/favorites
GET/POST       /api/data/reviews
```

---

### 2. Core Service — бизнес-логика

**Расположение:** [services/core-service/](services/core-service/)

Core Service содержит бизнес-логику приложения: валидацию данных, хеширование паролей, агрегацию данных из Data Service.

**Ключевые файлы:**
- [services/core-service/src/main.cpp](services/core-service/src/main.cpp) — бизнес-логика и обработка запросов
  - Строки 15-24: функция `hashPassword()` для SHA256 хеширования
  - Строки 50-90: регистрация пользователя с проверкой уникальности email
  - Строки 92-128: аутентификация с верификацией пароля
  - Строки 150-195: получение рецептов с фильтрацией и поиском
- [services/core-service/src/data_service_client.cpp](services/core-service/src/data_service_client.cpp) — HTTP-клиент для вызова Data Service
- [services/core-service/src/http_client.cpp](services/core-service/src/http_client.cpp) — базовый HTTP-клиент на libcurl
  - Строки 44-52: проверка HTTP статус-кодов для корректной обработки ошибок

**Эндпоинты Core Service:**
```
POST  /api/core/auth/register    — регистрация (валидация + хеширование)
POST  /api/core/auth/login       — аутентификация (проверка пароля)
GET   /api/core/auth/user/:id    — данные пользователя
GET   /api/core/recipes          — список рецептов с фильтрами
GET   /api/core/recipes/:id      — детали рецепта
GET   /api/core/categories       — категории
GET   /api/core/ingredients      — ингредиенты
```

---

### 3. Auth Service — аутентификация (Бонус #2)

**Расположение:** [services/auth-service/](services/auth-service/)

Auth Service выделен как **отдельный микросервис** для управления JWT-токенами. Это реализация бонусного задания #2 — вынос аутентификации в отдельный сервис.

**Ключевые файлы:**
- [services/auth-service/src/main.cpp](services/auth-service/src/main.cpp) — JWT генерация и верификация
  - Строки 20-35: структура `JwtConfig` с настройками токенов
  - Строки 37-60: `generateAccessToken()` — создание access token (24 часа)
  - Строки 62-80: `generateRefreshToken()` — создание refresh token (7 дней)
  - Строки 82-115: `verifyToken()` — верификация и декодирование JWT
  - Строки 130-165: эндпоинт `/auth/register` — регистрация через Core Service + генерация токенов
  - Строки 167-210: эндпоинт `/auth/login` — аутентификация через Core Service + генерация токенов
  - Строки 212-250: эндпоинт `/auth/verify` — внутренний эндпоинт для Gateway
  - Строки 252-290: эндпоинт `/auth/refresh` — обновление access token
- [services/auth-service/src/http_client.cpp](services/auth-service/src/http_client.cpp) — HTTP-клиент для вызова Core Service
- [services/auth-service/CMakeLists.txt](services/auth-service/CMakeLists.txt) — сборка с jwt-cpp и OpenSSL

**Эндпоинты Auth Service:**
| Endpoint | Метод | Описание |
|----------|-------|----------|
| `/auth/register` | POST | Регистрация → Core Service → генерация JWT |
| `/auth/login` | POST | Логин → Core Service → генерация JWT |
| `/auth/verify` | POST | Верификация токена (вызывается Gateway) |
| `/auth/refresh` | POST | Обновление access token по refresh token |
| `/auth/logout` | POST | Выход (stateless, очистка на клиенте) |
| `/auth/me` | GET | Данные текущего пользователя по токену |
| `/auth/health` | GET | Health check |

**Формат ответа при логине/регистрации:**
```json
{
  "access_token": "eyJhbGciOiJIUzI1NiIs...",
  "refresh_token": "eyJhbGciOiJIUzI1NiIs...",
  "token_type": "Bearer",
  "expires_in": 86400,
  "user": {
    "id": 1,
    "username": "user",
    "email": "user@example.com",
    "role": 1
  }
}
```

---

### 4. Gateway Service — внешний API

**Расположение:** [services/gateway-service/](services/gateway-service/)

Gateway Service — единая точка входа для внешних клиентов. Он маршрутизирует запросы к внутренним сервисам и выполняет аутентификацию через Auth Service.

**Ключевые файлы:**
- [services/gateway-service/src/main.cpp](services/gateway-service/src/main.cpp) — маршрутизация и middleware
  - Строки 45-75: функция `authenticateRequest()` — верификация токена через Auth Service
  - Строки 100-130: эндпоинты `/api/v2/auth/*` — проксирование в Auth Service
  - Строки 150-200: эндпоинты `/api/v2/recipes/*` — проксирование в Core Service
  - Строки 220-250: эндпоинты `/api/v2/categories`, `/api/v2/ingredients`
- [services/gateway-service/src/auth_service_client.cpp](services/gateway-service/src/auth_service_client.cpp) — клиент для Auth Service
  - `registerUser()`, `loginUser()`, `verifyToken()`, `refreshToken()`, `logout()`
- [services/gateway-service/src/core_service_client.cpp](services/gateway-service/src/core_service_client.cpp) — клиент для Core Service
- [services/gateway-service/include/auth_service_client.h](services/gateway-service/include/auth_service_client.h) — интерфейс AuthServiceClient

**Внешний API (через Caddy на порту 9000):**
```
POST  /api/v2/auth/register     — регистрация
POST  /api/v2/auth/login        — вход
POST  /api/v2/auth/refresh      — обновление токена
POST  /api/v2/auth/logout       — выход
GET   /api/v2/recipes           — список рецептов
GET   /api/v2/recipes/:id       — детали рецепта
GET   /api/v2/categories        — категории
GET   /api/v2/ingredients       — ингредиенты
GET   /api/v2/health            — проверка здоровья
```

---

### 5. Межсервисное взаимодействие

Все сервисы взаимодействуют через HTTP REST API с использованием libcurl.

```
┌─────────────┐     ┌──────────────┐     ┌──────────────┐     ┌──────────────┐     ┌────────────┐
│   Client    │────▶│   Gateway    │────▶│    Auth      │────▶│    Core      │────▶│    Data    │
│             │     │   Service    │     │   Service    │     │   Service    │     │   Service  │
└─────────────┘     └──────────────┘     └──────────────┘     └──────────────┘     └────────────┘
                           │                                         │                    │
                           │                                         │                    ▼
                           └─────────────────────────────────────────┘              ┌────────────┐
                                        (для не-auth запросов)                      │ PostgreSQL │
                                                                                    └────────────┘
```

**HTTP-клиенты:**
| Файл | Назначение |
|------|------------|
| [services/gateway-service/src/auth_service_client.cpp](services/gateway-service/src/auth_service_client.cpp) | Gateway → Auth |
| [services/gateway-service/src/core_service_client.cpp](services/gateway-service/src/core_service_client.cpp) | Gateway → Core |
| [services/auth-service/src/http_client.cpp](services/auth-service/src/http_client.cpp) | Auth → Core |
| [services/core-service/src/data_service_client.cpp](services/core-service/src/data_service_client.cpp) | Core → Data |

---

### 6. Docker Compose конфигурация

**Файл:** [docker-compose.yml](docker-compose.yml)

Каждый сервис запускается в 3 экземплярах для отказоустойчивости:

```yaml
# Data Service (строки 70-140)
data-service-1:
  build:
    context: .
    dockerfile: services/data-service/docker/Dockerfile
  environment:
    - DB_HOST=postgres-master
    - PORT=8082

# Core Service (строки 145-210)
core-service-1:
  environment:
    - DATA_SERVICE_URL=http://data-service-1:8082
    - PORT=8081

# Auth Service (строки 215-280)
auth-service-1:
  environment:
    - CORE_SERVICE_URL=http://core-service-1:8081
    - JWT_SECRET=your-secret-key
    - PORT=8083

# Gateway Service (строки 285-350)
gateway-service-1:
  environment:
    - AUTH_SERVICE_URL=http://auth-service-1:8083
    - CORE_SERVICE_URL=http://core-service-1:8081
    - PORT=8080
  ports:
    - "8080:8080"
```

---

### 7. Балансировка нагрузки (Caddy)

**Файл:** [docker/caddy/Caddyfile](docker/caddy/Caddyfile)

Caddy распределяет запросы между Gateway инстансами:
- **GET запросы**: распределение 2:1:1 (gateway-1 получает 50%)
- **Мутирующие запросы** (POST/PUT/DELETE): направляются на gateway-1

```caddyfile
@read_requests {
    method GET HEAD OPTIONS
}

@write_requests {
    method POST PUT PATCH DELETE
}

handle @read_requests {
    reverse_proxy gateway-service-1:8080 gateway-service-2:8080 gateway-service-3:8080 {
        lb_policy weighted_round_robin 2 1 1
    }
}

handle @write_requests {
    reverse_proxy gateway-service-1:8080
}
```

---

### 8. Логирование и мониторинг

**Файл:** [docker/promtail/promtail-config.yml](docker/promtail/promtail-config.yml)

Promtail собирает логи всех контейнеров и добавляет метки для фильтрации в Grafana:

```yaml
relabel_configs:
  # Определение типа сервиса
  - source_labels: [__meta_docker_container_name]
    regex: '.*gateway-service.*'
    replacement: 'gateway'
    target_label: service_type
  - source_labels: [__meta_docker_container_name]
    regex: '.*auth-service.*'
    replacement: 'auth'
    target_label: service_type
  - source_labels: [__meta_docker_container_name]
    regex: '.*core-service.*'
    replacement: 'core'
    target_label: service_type
  - source_labels: [__meta_docker_container_name]
    regex: '.*data-service.*'
    replacement: 'data'
    target_label: service_type
```

**Запросы для Grafana (Loki):**
```
{service_type="auth"}                              # логи Auth Service
{service_type=~"gateway|auth|core|data"}           # все микросервисы
{container_name=~".*service.*"} | line_format "{{.container_name}}: {{__line__}}"
```

---

### 9. Исправления для совместимости

В процессе интеграции были внесены исправления:

**[libs/core/src/domain/entities/user.cpp](libs/core/src/domain/entities/user.cpp)** — исправлен `userRoleToString()`:
```cpp
QString userRoleToString(UserRole role) {
    switch (role) {
    case UserRole::GUEST: return "user";  // PostgreSQL has no 'guest'
    case UserRole::USER: return "user";
    case UserRole::ADMIN: return "admin";
    default: return "user";
    }
}
```
PostgreSQL enum `user_role` содержит только `'user'` и `'admin'`, поэтому `GUEST` маппится на `'user'`.

**[services/core-service/src/http_client.cpp](services/core-service/src/http_client.cpp)** — добавлена проверка HTTP статус-кодов:
```cpp
long httpCode = 0;
curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
if (httpCode >= 400) {
    response = "";  // Return empty for error responses
}
```

---

## Структура файлов

```
services/
├── data-service/
│   ├── src/main.cpp              # REST API над репозиториями
│   ├── CMakeLists.txt
│   └── docker/Dockerfile
├── core-service/
│   ├── src/
│   │   ├── main.cpp              # Бизнес-логика
│   │   ├── data_service_client.cpp
│   │   └── http_client.cpp
│   ├── include/
│   │   ├── data_service_client.h
│   │   └── http_client.h
│   ├── CMakeLists.txt
│   └── docker/Dockerfile
├── auth-service/                 # НОВЫЙ СЕРВИС (Бонус #2)
│   ├── src/
│   │   ├── main.cpp              # JWT-аутентификация
│   │   └── http_client.cpp
│   ├── include/
│   │   └── http_client.h
│   ├── CMakeLists.txt
│   └── docker/Dockerfile
└── gateway-service/
    ├── src/
    │   ├── main.cpp              # Внешний API, маршрутизация
    │   ├── core_service_client.cpp
    │   └── auth_service_client.cpp  # НОВЫЙ клиент
    ├── include/
    │   ├── core_service_client.h
    │   └── auth_service_client.h    # НОВЫЙ заголовок
    ├── CMakeLists.txt
    └── docker/Dockerfile
```

---

## Быстрый старт

```bash
# Запуск всех сервисов
./scripts/lab6_start.sh

# Проверка здоровья
curl http://localhost:9000/api/v2/health

# Регистрация пользователя
curl -X POST http://localhost:9000/api/v2/auth/register \
  -H "Content-Type: application/json" \
  -d '{"username":"testuser","email":"test@example.com","password":"password123"}'

# Вход
curl -X POST http://localhost:9000/api/v2/auth/login \
  -H "Content-Type: application/json" \
  -d '{"email":"test@example.com","password":"password123"}'

# Запрос с токеном
curl http://localhost:9000/api/v2/recipes \
  -H "Authorization: Bearer <access_token>"

# Остановка
./scripts/lab6_stop.sh
```

---

## Эндпоинты

| URL | Описание |
|-----|----------|
| http://localhost:9000 | Основное приложение (через Caddy) |
| http://localhost:9000/mirror | Зеркальное приложение |
| http://localhost:9000/api/v2/* | REST API |
| http://localhost:8080/api/v2/* | Прямой доступ к Gateway 1 |
| http://localhost:3000 | Grafana (admin/admin) |

---

## Скрипты

| Скрипт | Описание |
|--------|----------|
| [scripts/lab6_start.sh](scripts/lab6_start.sh) | Сборка и запуск всех сервисов |
| [scripts/lab6_stop.sh](scripts/lab6_stop.sh) | Остановка сервисов |
| [scripts/lab6_restart-clean.sh](scripts/lab6_restart-clean.sh) | Полный перезапуск с очисткой volumes |

---

## Технологии

- **Язык**: C++17
- **HTTP фреймворк**: Crow
- **HTTP клиент**: libcurl
- **JWT**: jwt-cpp
- **JSON**: nlohmann/json
- **База данных**: PostgreSQL 16 (master-slave репликация)
- **Load Balancer**: Caddy 2
- **Контейнеризация**: Docker, Docker Compose
- **Мониторинг**: Grafana + Loki + Promtail

---

## Демонстрация функциональности

### 1. Проверка запущенных контейнеров

```bash
# Показать все контейнеры и их статус
docker-compose ps

# Только запущенные сервисы
docker-compose ps --filter "status=running"

# Подсчёт контейнеров (ожидается 18)
docker-compose ps -q | wc -l
```

### 2. Health-check всех сервисов

```bash
# Health через Caddy (балансировщик)
curl -s http://localhost:9000/api/v2/health | jq

# Прямой health-check Gateway инстансов
curl -s http://localhost:8080/api/v2/health   # gateway-1
curl -s http://localhost:8081/api/v2/health   # gateway-2
curl -s http://localhost:8082/api/v2/health   # gateway-3
```

### 3. Демонстрация аутентификации

```bash
# Регистрация нового пользователя
curl -X POST http://localhost:9000/api/v2/auth/register \
  -H "Content-Type: application/json" \
  -d '{"username":"demo","email":"demo@test.com","password":"demo123"}' | jq

# Вход (получение JWT токенов)
RESPONSE=$(curl -s -X POST http://localhost:9000/api/v2/auth/login \
  -H "Content-Type: application/json" \
  -d '{"email":"demo@test.com","password":"demo123"}')
echo $RESPONSE | jq

# Извлечение access_token
TOKEN=$(echo $RESPONSE | jq -r '.access_token')
echo "Access Token: $TOKEN"

# Обновление токена
REFRESH=$(echo $RESPONSE | jq -r '.refresh_token')
curl -X POST http://localhost:9000/api/v2/auth/refresh \
  -H "Content-Type: application/json" \
  -d "{\"refresh_token\":\"$REFRESH\"}" | jq
```

### 4. Работа с рецептами

```bash
# Список всех рецептов
curl -s http://localhost:9000/api/v2/recipes | jq

# Детали рецепта (id=1)
curl -s http://localhost:9000/api/v2/recipes/1 | jq

# Поиск рецептов
curl -s "http://localhost:9000/api/v2/recipes?search=салат" | jq

# Фильтрация по категории
curl -s "http://localhost:9000/api/v2/recipes?category=1" | jq
```

### 5. Категории и ингредиенты

```bash
# Список категорий
curl -s http://localhost:9000/api/v2/categories | jq

# Список ингредиентов
curl -s http://localhost:9000/api/v2/ingredients | jq
```

### 6. Демонстрация балансировки нагрузки

```bash
# Скрипт отправляет 20 GET-запросов и показывает распределение
for i in {1..20}; do
  curl -s http://localhost:9000/api/v2/health | jq -r '.instance // .service // "unknown"'
done | sort | uniq -c

# Или использовать готовый скрипт
./scripts/lab6_test-load-balancing.sh
```

### 7. Просмотр логов

```bash
# Логи всех сервисов (последние 50 строк)
docker-compose logs --tail=50

# Логи конкретного сервиса
docker-compose logs gateway-service-1 --tail=30
docker-compose logs auth-service-1 --tail=30
docker-compose logs core-service-1 --tail=30
docker-compose logs data-service-1 --tail=30

# Логи в реальном времени
docker-compose logs -f gateway-service-1 auth-service-1

# Логи всех инстансов одного сервиса
docker-compose logs gateway-service-1 gateway-service-2 gateway-service-3
```

### 8. Мониторинг в Grafana

1. Открыть http://localhost:3000 (admin/admin)
2. Перейти в **Explore** → выбрать **Loki**
3. Полезные запросы:

```
# Все логи микросервисов
{service_type=~"gateway|auth|core|data"}

# Только Gateway
{service_type="gateway"}

# Только Auth Service
{service_type="auth"}

# Поиск ошибок
{service_type=~"gateway|auth|core|data"} |~ "error|Error|ERROR"

# Логи с именем контейнера
{job=~".*service.*"} | line_format "{{.job}}: {{__line__}}"
```

### 9. Межсервисное взаимодействие (трассировка)

```bash
# Отправить запрос и наблюдать цепочку вызовов в логах
# Терминал 1: наблюдение за логами
docker-compose logs -f gateway-service-1 auth-service-1 core-service-1 data-service-1

# Терминал 2: отправить запрос на логин
curl -X POST http://localhost:9000/api/v2/auth/login \
  -H "Content-Type: application/json" \
  -d '{"email":"demo@test.com","password":"demo123"}'

# В логах будет видна цепочка:
# gateway-service-1: POST /api/v2/auth/login
# auth-service-1: POST /auth/login -> calling core-service
# core-service-1: POST /api/core/auth/login -> calling data-service
# data-service-1: GET /api/data/users/email/demo@test.com
```

### 10. Проверка репликации PostgreSQL

```bash
# Статус master
docker exec postgres-master psql -U admin -d fridgefusion \
  -c "SELECT * FROM pg_stat_replication;"

# Количество записей на master и slave
docker exec postgres-master psql -U admin -d fridgefusion \
  -c "SELECT COUNT(*) as recipes_count FROM recipes;"
docker exec postgres-slave psql -U admin -d fridgefusion \
  -c "SELECT COUNT(*) as recipes_count FROM recipes;"
```

### 11. CLI демонстрация

```bash
# Сборка CLI (если ещё не собран)
cd cli && mkdir -p build && cd build && cmake .. && make && cd ../..

# Запуск CLI
./cli/build/ff

# Команды в CLI:
# > login demo@test.com demo123
# > recipes
# > recipe 1
# > categories
# > help
# > exit
```

### 12. Полный цикл демонстрации

```bash
#!/bin/bash
# Демонстрационный сценарий

echo "=== 1. Проверка контейнеров ==="
docker-compose ps | head -20

echo -e "\n=== 2. Health check ==="
curl -s http://localhost:9000/api/v2/health | jq

echo -e "\n=== 3. Регистрация пользователя ==="
curl -s -X POST http://localhost:9000/api/v2/auth/register \
  -H "Content-Type: application/json" \
  -d '{"username":"testuser","email":"test@demo.com","password":"pass123"}' | jq

echo -e "\n=== 4. Вход ==="
RESP=$(curl -s -X POST http://localhost:9000/api/v2/auth/login \
  -H "Content-Type: application/json" \
  -d '{"email":"test@demo.com","password":"pass123"}')
echo $RESP | jq
TOKEN=$(echo $RESP | jq -r '.access_token')

echo -e "\n=== 5. Получение рецептов ==="
curl -s http://localhost:9000/api/v2/recipes | jq '.[0:2]'

echo -e "\n=== 6. Детали рецепта ==="
curl -s http://localhost:9000/api/v2/recipes/1 | jq

echo -e "\n=== 7. Категории ==="
curl -s http://localhost:9000/api/v2/categories | jq

echo -e "\n=== Демонстрация завершена ==="
```
