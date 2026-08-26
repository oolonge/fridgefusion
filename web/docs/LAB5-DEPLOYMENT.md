# FridgeFusion Lab #5 - Deployment Guide

## WebLab#5: WEB SERVER Senior DevOps

Реализация лабораторной работы #5 с балансировкой нагрузки, репликацией БД, кешированием и мониторингом.

---

## Реализованные требования

### Основные задания

- ✅ **Балансировка GET-запросов** в соотношении 2:1:1 на 3 бэкенда
- ✅ **POST/PUT/PATCH/DELETE** только на master-инстанс с правами на запись
- ✅ **Обработка ошибок** при попытке записи в read-only инстансы (на уровне роутинга)
- ✅ **PostgreSQL репликация** (master-slave)
- ✅ **Маршрут /mirror** с полноценной версией приложения
- ✅ **Подмена Server header** на "FridgeFusion/2.0 (Lab#5)"
- ✅ **Кеширование** GET-запросов (кроме /api)
- ✅ **gzip-сжатие** для всех ответов
- ✅ **Мониторинг** через Grafana + Loki (логи со всех 6 инстансов: 3×main + 3×mirror)

### Бонусы

- ✅ **Bonus #1**: HTTPS с self-signed сертификатом и HTTP/2
- ✅ **Bonus #3**: HTTP Health Checks для всех бэкендов

---

## Архитектура

```
                            ┌─────────────────────┐
                            │   Caddy :8443       │
                            │   (HTTPS/HTTP2)     │
                            │   Load Balancer     │
                            └──────────┬──────────┘
                                       │
                    ┌──────────────────┴──────────────────┐
                    │                                      │
         ┌──────────▼──────────┐              ┌──────────▼──────────┐
         │  Main App (/)       │              │  Mirror (/mirror)   │
         │                     │              │                     │
         │  GET: 2:1:1         │              │  GET: 2:1:1         │
         │  - Master :8080 (×2)│              │  - Master :8090 (×2)│
         │  - RO1 :8081        │              │  - RO1 :8091        │
         │  - RO2 :8082        │              │  - RO2 :8092        │
         │                     │              │                     │
         │  POST/PUT/DELETE    │              │  POST/PUT/DELETE    │
         │  → Master only      │              │  → Master only      │
         └──────────┬──────────┘              └──────────┬──────────┘
                    │                                    │
         ┌──────────▼──────────┐              ┌──────────▼──────────┐
         │  PostgreSQL Master  │◄─replication─┤  PostgreSQL Slave   │
         │  :5432 (R/W)        │              │  :5433 (R/O)        │
         └─────────────────────┘              └─────────────────────┘

                    ┌──────────────────────────────┐
                    │  Monitoring Stack            │
                    │  - Grafana :3000             │
                    │  - Loki :3100                │
                    │  - Promtail (log shipper)    │
                    └──────────────────────────────┘
```

---

## Требования

- Docker 20.10+
- Docker Compose 1.29+
- 4GB RAM минимум
- 10GB свободного места

---

## Быстрый старт

### 1. Запуск всех сервисов

```bash
# Дать права на выполнение скриптов (если нужно)
chmod +x start.sh stop.sh test-load-balancing.sh

# Запустить все сервисы
./start.sh
```

Первый запуск займет 10-15 минут на сборку C++ бэкенда.

### 2. Проверка работы

После запуска доступны следующие endpoints:

**Основное приложение:**
- HTTP: http://localhost:9000
- HTTPS: https://localhost:9443 ⚠️ (примите self-signed сертификат)

**Зеркало:**
- HTTP: http://localhost:9000/mirror
- HTTPS: https://localhost:9443/mirror

**API:**
- Main API: https://localhost:9443/api/v2/health
- Mirror API: https://localhost:9443/mirror/api/v2/health
- Swagger: https://localhost:9443/api/docs

**Мониторинг:**
- Grafana: http://localhost:3000 (логин: `admin`, пароль: `admin`)
- Loki: http://localhost:3100

**Базы данных:**
- Master (R/W): `localhost:5432`
- Slave (R/O): `localhost:5433`
- Пользователь: `postgres`
- Пароль: `postgres_secret`
- База: `fridgefusion`

### 3. Тестирование балансировки

```bash
# Запустить тест балансировки
./test-load-balancing.sh
```

### 4. Просмотр логов

```bash
# Все сервисы
docker-compose logs -f

# Конкретный сервис
docker-compose logs -f caddy
docker-compose logs -f backend-master
docker-compose logs -f postgres-master

# Только последние 100 строк
docker-compose logs --tail=100 caddy
```

### 5. Остановка

```bash
# Остановить сервисы
./stop.sh

# Или остановить и удалить volumes
docker-compose down -v
```

---

## Проверка требований

### 1. Балансировка GET-запросов (2:1:1)

```bash
# Отправить 100 GET запросов
for i in {1..100}; do
  curl -sk https://localhost:9443/api/v2/health > /dev/null
  sleep 0.1
done

# Проверить распределение в логах Caddy
docker-compose logs caddy | grep -E "8080|8081|8082" | \
  awk '{print $NF}' | sort | uniq -c
```

**Ожидаемый результат:** ~50% на 8080 (master), ~25% на 8081, ~25% на 8082

### 2. POST/PUT/DELETE только на master

```bash
# Попытка создать пользователя
curl -X POST https://localhost:9443/api/v2/auth/register \
  -H "Content-Type: application/json" \
  -d '{"username":"test","email":"test@test.com","password":"test123"}' \
  -k -v

# Проверить логи - запрос должен быть только на master
docker-compose logs backend-master | grep "POST.*register"
docker-compose logs backend-ro1 | grep "POST.*register"  # пусто
docker-compose logs backend-ro2 | grep "POST.*register"  # пусто
```

### 3. Репликация PostgreSQL

```bash
# На master (должна быть запись)
docker exec -it fridgefusion-postgres-master psql -U postgres -d fridgefusion \
  -c "SELECT * FROM pg_stat_replication;"

# На slave (должно быть видно recovery mode)
docker exec -it fridgefusion-postgres-slave psql -U postgres -d fridgefusion \
  -c "SELECT pg_is_in_recovery();"
```

**Ожидаемый результат:** slave в recovery mode (true)

### 4. Read-only пользователь

```bash
# Попытка записи на slave с readonly_user должна упасть
docker exec -it fridgefusion-postgres-slave psql \
  -U readonly_user -d fridgefusion \
  -c "INSERT INTO categories (name) VALUES ('test');"
```

**Ожидаемый результат:** ERROR: permission denied

### 5. Маршрут /mirror работает

```bash
# Main app
curl -sk https://localhost:9443/api/v2/health
curl -sk https://localhost:9443/

# Mirror app
curl -sk https://localhost:9443/mirror/api/v2/health
curl -sk https://localhost:9443/mirror/
```

### 6. Кеширование и gzip

```bash
# Проверить заголовки кеша
curl -sk -I https://localhost:9443/styles/common.css | grep "Cache-Control"

# Проверить gzip
curl -sk -I https://localhost:9443/api/v2/health | grep "Content-Encoding"
```

**Ожидаемые заголовки:**
- `Cache-Control: public, max-age=3600` (для статики)
- `Content-Encoding: gzip`

### 7. Custom Server header

```bash
curl -sk -I https://localhost:9443/ | grep "Server"
```

**Ожидаемый результат:** `Server: FridgeFusion/2.0 (Lab#5)`

### 8. HTTPS и HTTP/2

```bash
# Проверить протокол
curl -Isk https://localhost:9443/ | head -1
```

**Ожидаемый результат:** `HTTP/2 200`

### 9. Health Checks

```bash
# Проверить health check в конфиге Caddy
cat docker/caddy/Caddyfile | grep -A3 "health_uri"

# Остановить один backend и проверить, что балансировщик его исключит
docker stop fridgefusion-backend-ro1
sleep 15
./test-load-balancing.sh
# Все запросы должны идти только на 8080 и 8082

# Запустить обратно
docker start fridgefusion-backend-ro1
```

### 10. Мониторинг логов (Grafana + Loki)

1. Откройте http://localhost:3000
2. Логин: `admin`, пароль: `admin`
3. Перейдите в Explore
4. Выберите data source: Loki
5. Используйте запрос:
   ```
   {job=~"backend.*"}
   ```
6. Должны быть видны логи со всех 6 бэкендов

---

## Нагрузочное тестирование

См. [LOAD-TEST-REPORT.md](./LOAD-TEST-REPORT.md) для результатов ApacheBench тестирования.

Для запуска тестов вручную:

```bash
# Установить ApacheBench (если нужно)
# Ubuntu/Debian:
sudo apt-get install apache2-utils

# macOS:
brew install httpd

# Простой тест
ab -n 1000 -c 10 https://localhost:9443/api/v2/health

# Подробный тест с отчётом
ab -n 10000 -c 100 -g results.tsv https://localhost:9443/api/v2/recipes
```

---

## Устранение неполадок

### Проблема: Контейнеры не стартуют

```bash
# Проверить логи
docker-compose logs

# Пересоздать volumes
docker-compose down -v
./start.sh
```

### Проблема: Slave не реплицируется

```bash
# Проверить статус репликации на master
docker exec -it fridgefusion-postgres-master psql -U postgres \
  -c "SELECT * FROM pg_stat_replication;"

# Пересоздать slave
docker-compose stop postgres-slave
docker volume rm web-dev_postgres-slave-data
docker-compose up -d postgres-slave
```

### Проблема: Backend не собирается

```bash
# Пересобрать с нуля
docker-compose build --no-cache backend-master

# Проверить логи сборки
docker-compose build backend-master 2>&1 | tee build.log
```

---

## Структура проекта

```
Web-dev/
├── docker-compose.yml              # Главная конфигурация
├── start.sh                        # Скрипт запуска
├── stop.sh                         # Скрипт остановки
├── test-load-balancing.sh          # Тест балансировки
├── LAB5-DEPLOYMENT.md              # Эта документация
├── LOAD-TEST-REPORT.md             # Отчёт по нагрузке (создать отдельно)
│
├── docker/
│   ├── backend/
│   │   ├── Dockerfile              # Multi-stage build для C++ бэкенда
│   │   ├── config-master.json      # Конфиг master бэкенда
│   │   ├── config-ro1.json         # Конфиг read-only 1
│   │   ├── config-ro2.json         # Конфиг read-only 2
│   │   ├── config-mirror-*.json    # Конфиги для mirror
│   │
│   ├── caddy/
│   │   └── Caddyfile               # Балансировщик + reverse proxy
│   │
│   ├── postgres/
│   │   ├── init-db.sql             # Схема БД + тестовые данные
│   │   ├── master/
│   │   │   ├── init.sh             # Инициализация master
│   │   │   ├── postgresql.conf    # Конфиг master
│   │   │   └── pg_hba.conf        # Права доступа
│   │   └── slave/
│   │       ├── init.sh             # Настройка репликации
│   │       └── postgresql.conf    # Конфиг slave
│   │
│   ├── loki/
│   │   └── loki-config.yml         # Loki для агрегации логов
│   │
│   ├── promtail/
│   │   └── promtail-config.yml     # Promtail для отправки логов
│   │
│   └── grafana/
│       ├── datasources.yml         # Loki datasource
│       └── dashboards.yml          # Dashboards provisioning
│
├── web_api/                        # C++ Backend (Crow)
├── libs/                           # Библиотеки (core, application, etc)
├── web_server/                     # Статические файлы
└── third_party/                    # Зависимости (Crow, JWT, JSON)
```

---

## Тестовые данные

После инициализации БД доступны тестовые пользователи:

| Username | Email | Password | Role |
|----------|-------|----------|------|
| admin | admin@fridgefusion.com | password | admin |
| testuser | test@fridgefusion.com | password | user |
| john_doe | john@example.com | password | user |

⚠️ **Важно:** Пароль хешируется с bcrypt. В текущей реализации используется хеш для "password".

---

## Полезные команды

```bash
# Посмотреть статус всех контейнеров
docker-compose ps

# Рестарт конкретного сервиса
docker-compose restart backend-master

# Выполнить команду в контейнере
docker exec -it fridgefusion-backend-master /app/fridgefusion_api

# Посмотреть использование ресурсов
docker stats

# Очистить всё (volumes, images, containers)
docker-compose down -v
docker system prune -a
```

---

## Автор

Lab #5 - FridgeFusion
Season: 2025
