# FridgeFusion Lab #5 - Checklist для проверки

## Быстрая проверка (5 минут)

### 1. Запуск системы

```bash
cd /Users/administrator/Desktop/Web/FridgeFusion
./start.sh
```

Ожидаемый результат: Все 11 сервисов запущены и здоровы.

### 2. Базовая проверка работоспособности

**⚠️ Примечание для macOS:** LibreSSL не работает с self-signed сертификатами Caddy. Используйте HTTP (порт 9000) или браузер.

```bash
# Вариант 1: HTTP (работает везде)
curl http://localhost:9000/api/v2/health
# Ожидаемый результат: {"status":"ok","service":"FridgeFusion API","version":"2.0.0"}

curl http://localhost:9000/mirror/api/v2/health
# Ожидаемый результат: {"status":"ok","service":"FridgeFusion API","version":"2.0.0"}

# Вариант 2: HTTPS (для Linux или с curl-openssl на macOS)
curl -k https://localhost:9443/api/v2/health
curl -k https://localhost:9443/mirror/api/v2/health

# Server header (через HTTP)
curl -I http://localhost:9000/ | grep Server
# Ожидаемый результат: Server: FridgeFusion/2.0 (Lab#5)

# HTTP/2 (только через HTTPS в браузере)
open https://localhost:9443/
# Примите self-signed сертификат, затем проверьте DevTools → Network → Protocol
```

### 3. Проверка балансировки

```bash
./test-load-balancing.sh
```

Затем проверить логи:
```bash
docker compose logs caddy --tail=100 | grep -E "backend-(master|ro1|ro2)" | \
  grep -oE ":(8080|8081|8082)" | sort | uniq -c
```

Ожидаемое распределение (примерно): ~50% на 8080, ~25% на 8081, ~25% на 8082

### 4. Проверка мониторинга

Откройте браузер: http://localhost:3000
- Логин: `admin`, пароль: `admin`
- Explore → выберите Loki
- Запрос: `{job=~"backend.*"}`
- Должны быть видны логи со всех 6 бэкендов

---

## Детальная проверка требований (20 минут)

### ✅ Требование 1: Балансировка GET 2:1:1

**Команда:**
```bash
# Отправить 300 GET запросов
for i in {1..300}; do
  curl -sk https://localhost:9443/api/v2/health > /dev/null
  sleep 0.05
done

# Проверить распределение
docker compose logs caddy --tail=300 | \
  grep "GET /api/v2/health" | \
  grep -oE "backend-(master|ro1|ro2):[0-9]+" | \
  sort | uniq -c
```

**Ожидаемый результат:**
```
~150 backend-master:8080   (50%)
 ~75 backend-ro1:8081      (25%)
 ~75 backend-ro2:8082      (25%)
```

**Конфигурация:** `docker/caddy/Caddyfile:26-47`

---

### ✅ Требование 2: POST/PUT/DELETE только на master

**Команда:**
```bash
# Попытка создать пользователя (POST)
curl -X POST https://localhost:9443/api/v2/auth/register \
  -H "Content-Type: application/json" \
  -d '{"username":"testuser999","email":"test999@test.com","password":"test123"}' \
  -k -v 2>&1 | grep -i "HTTP\|Server"

# Проверить логи
docker compose logs backend-master --tail=50 | grep "POST.*register"
docker compose logs backend-ro1 --tail=50 | grep "POST.*register"
docker compose logs backend-ro2 --tail=50 | grep "POST.*register"
```

**Ожидаемый результат:**
- POST запрос есть в логах `backend-master`
- POST запроса НЕТ в логах `backend-ro1` и `backend-ro2`

**Конфигурация:** `docker/caddy/Caddyfile:49-63`

---

### ✅ Требование 3: Обработка ошибок read-only

**Объяснение:**
Ошибки обрабатываются на уровне Caddy — write-запросы не маршрутизируются на read-only инстансы.

**Дополнительная проверка (прямой запрос к ro-инстансу):**
```bash
# Минуя балансировщик, напрямую к ro1
curl -X POST http://localhost:8081/api/v2/recipes \
  -H "Content-Type: application/json" \
  -d '{"name":"Test"}' \
  -v
```

**Ожидаемый результат:** HTTP 500 с ошибкой доступа к БД (PostgreSQL вернёт permission denied)

---

### ✅ Требование 4: PostgreSQL master-slave репликация

**Команда:**
```bash
# Проверка репликации на master
docker exec -it fridgefusion-postgres-master psql -U postgres -d fridgefusion \
  -c "SELECT client_addr, state, sync_state FROM pg_stat_replication;"

# Проверка slave в recovery mode
docker exec -it fridgefusion-postgres-slave psql -U postgres -d fridgefusion \
  -c "SELECT pg_is_in_recovery();"
```

**Ожидаемый результат:**
- Master: показывает подключение от slave (state: streaming)
- Slave: `pg_is_in_recovery` возвращает `t` (true)

**Тест репликации:**
```bash
# Добавить данные на master
docker exec -it fridgefusion-postgres-master psql -U postgres -d fridgefusion \
  -c "INSERT INTO categories (name, description) VALUES ('Test Category', 'Test');"

# Проверить на slave (данные должны появиться в течение 1-2 секунд)
docker exec -it fridgefusion-postgres-slave psql -U readonly_user -d fridgefusion \
  -c "SELECT * FROM categories WHERE name='Test Category';"
```

**Конфигурация:**
- `docker/postgres/master/postgresql.conf` — wal_level=replica
- `docker/postgres/slave/init.sh` — pg_basebackup + standby.signal

---

### ✅ Требование 5: Маршрут /mirror

**Команда:**
```bash
# Проверить API mirror
curl -sk https://localhost:9443/mirror/api/v2/health

# Проверить статику mirror
curl -sk https://localhost:9443/mirror/ | grep -i "FridgeFusion"

# Проверить балансировку mirror (300 запросов)
for i in {1..300}; do
  curl -sk https://localhost:9443/mirror/api/v2/health > /dev/null
  sleep 0.05
done

docker compose logs caddy --tail=300 | \
  grep "mirror/api/v2/health" | \
  grep -oE "backend-mirror-(master|ro1|ro2):[0-9]+" | \
  sort | uniq -c
```

**Ожидаемый результат:**
- Mirror API работает
- Балансировка mirror: ~50% master, ~25% ro1, ~25% ro2
- Всего 6 backend инстансов работают (3 main + 3 mirror)

**Конфигурация:** `docker/caddy/Caddyfile:125-210`

---

### ✅ Требование 6: Server header, кеширование, gzip

**Команда:**
```bash
# Server header
curl -Ik https://localhost:9443/ | grep "Server"
# Ожидается: Server: FridgeFusion/2.0 (Lab#5)

# Кеширование статики
curl -Ik https://localhost:9443/styles/common.css | grep "Cache-Control"
# Ожидается: Cache-Control: public, max-age=3600

# gzip сжатие
curl -Ik https://localhost:9443/api/v2/recipes | grep "Content-Encoding"
# Ожидается: Content-Encoding: gzip

# Проверка, что /api НЕ кешируется
curl -Ik https://localhost:9443/api/v2/health | grep "Cache-Control"
# Не должно быть Cache-Control
```

**Конфигурация:**
- Server header: `docker/caddy/Caddyfile:24`
- Кеширование: `docker/caddy/Caddyfile:71-77`
- gzip: `docker/caddy/Caddyfile:22`

---

### ✅ Требование 7: Grafana + Loki (логи с 6 инстансов)

**Шаги:**
1. Открыть http://localhost:3000
2. Логин: `admin`, пароль: `admin`
3. Перейти в **Explore** (иконка компаса слева)
4. Убедиться, что выбран datasource **Loki**
5. Ввести запрос:
   ```
   {job=~"backend.*"}
   ```
6. Нажать **Run query**

**Ожидаемый результат:**
Должны быть видны логи от всех 6 jobs:
- backend-master
- backend-ro1
- backend-ro2
- backend-mirror-master
- backend-mirror-ro1
- backend-mirror-ro2

**Дополнительные запросы:**
```
{job="caddy"}                    # Логи Caddy
{job="backend-master"}           # Только master
{job=~"backend-mirror.*"}        # Только mirror инстансы
{job=~"backend.*"} |= "GET"      # Все GET запросы
{job=~"backend.*"} |= "POST"     # Все POST запросы (только master)
```

**Конфигурация:**
- `docker/loki/loki-config.yml`
- `docker/promtail/promtail-config.yml` — 7 scrape jobs
- `docker/grafana/datasources.yml`

---

## Бонусы

### ✅ Bonus #1: HTTPS + HTTP/2

**Команда:**
```bash
# Проверить протокол
curl -Ik https://localhost:9443/ | head -1
# Ожидается: HTTP/2 200

# Проверить сертификат (self-signed)
openssl s_client -connect localhost:9443 -showcerts </dev/null 2>/dev/null | \
  grep "subject="
```

**Конфигурация:**
- `docker/caddy/Caddyfile:10` — auto_https off
- `docker/caddy/Caddyfile:16` — tls internal

---

### ✅ Bonus #3: HTTP Health Checks

**Команда:**
```bash
# Остановить ro1
docker stop fridgefusion-backend-ro1

# Подождать, пока health check обнаружит падение (10-15 сек)
sleep 15

# Отправить 100 запросов
for i in {1..100}; do
  curl -sk https://localhost:9443/api/v2/health > /dev/null
done

# Проверить распределение — ro1 должен отсутствовать
docker compose logs caddy --tail=100 | \
  grep "api/v2/health" | \
  grep -oE ":(8080|8081|8082)" | \
  sort | uniq -c

# Запустить ro1 обратно — автоматически вернётся в балансировку
docker start fridgefusion-backend-ro1
sleep 15

# Проверить, что ro1 снова в ротации
for i in {1..100}; do
  curl -sk https://localhost:9443/api/v2/health > /dev/null
done

docker compose logs caddy --tail=100 | \
  grep "api/v2/health" | \
  grep -oE ":(8080|8081|8082)" | \
  sort | uniq -c
```

**Ожидаемый результат:**
- После остановки ro1: запросы только на 8080 и 8082
- После запуска ro1: запросы снова распределяются 2:1:1

**Конфигурация:** `docker/caddy/Caddyfile:36-42`

---

## Нагрузочное тестирование (опционально)

### Установка ApacheBench

```bash
# Ubuntu/Debian
sudo apt-get install apache2-utils

# macOS
brew install httpd
```

### Базовый тест

```bash
# 10,000 запросов, 100 одновременно
ab -n 10000 -c 100 -k https://localhost:9443/api/v2/health
```

**Ожидаемый результат:**
- Failed requests: 0
- Requests per second: [зависит от системы]

Подробности в [LOAD-TEST-REPORT.md](./LOAD-TEST-REPORT.md)

---

## Структура проекта (ключевые файлы)

```
Web-dev/
├── docker compose.yml                # Главная конфигурация (11 сервисов)
├── start.sh                          # Скрипт запуска
├── stop.sh                           # Скрипт остановки
├── test-load-balancing.sh            # Тест балансировки
│
├── docker/
│   ├── caddy/Caddyfile              # ★ Балансировщик + reverse proxy
│   ├── postgres/
│   │   ├── init-db.sql              # ★ Схема БД + тестовые данные
│   │   ├── master/                  # ★ Конфиги master PostgreSQL
│   │   └── slave/                   # ★ Конфиги slave PostgreSQL
│   ├── backend/
│   │   ├── Dockerfile               # ★ Multi-stage build C++ бэкенда
│   │   └── config-*.json            # ★ Конфиги 6 бэкендов
│   ├── loki/loki-config.yml         # ★ Log aggregation
│   ├── promtail/promtail-config.yml # ★ Log shipping (6 backends)
│   └── grafana/datasources.yml      # ★ Grafana datasource
│
└── Документация:
    ├── QUICKSTART.md                 # Быстрый старт (5 минут)
    ├── LAB5-DEPLOYMENT.md            # Полное руководство
    ├── LAB5-SUMMARY.md               # Резюме реализации
    ├── LAB5-CHECKLIST.md             # Этот файл
    └── LOAD-TEST-REPORT.md           # Шаблон отчёта
```

---

## Возможные проблемы и решения

### Проблема: "Cannot connect to Docker daemon"

**Решение:**
```bash
# Запустить Docker
sudo systemctl start docker  # Linux
# или запустить Docker Desktop на macOS/Windows
```

### Проблема: Контейнеры не стартуют / долго собираются

**Решение:**
```bash
# Пересобрать с нуля
docker compose down -v
docker compose build --no-cache
./start.sh
```

### Проблема: Slave не реплицируется

**Решение:**
```bash
# Пересоздать slave
docker compose stop postgres-slave
docker volume rm fridgefusion_postgres-slave-data
docker compose up -d postgres-slave
```

### Проблема: Health checks не работают

**Проверка:**
```bash
# Проверить, что /api/v2/health endpoint доступен
curl http://localhost:8080/api/v2/health
curl http://localhost:8081/api/v2/health
curl http://localhost:8082/api/v2/health
curl http://localhost:8090/api/v2/health
curl http://localhost:8091/api/v2/health
curl http://localhost:8092/api/v2/health

# Проверить, что backend'ы используют правильные порты из API_PORT
docker logs fridgefusion-backend-ro1 2>&1 | grep "API Server.*8081"
docker logs fridgefusion-backend-ro2 2>&1 | grep "API Server.*8082"
docker logs fridgefusion-backend-mirror-master 2>&1 | grep "API Server.*8090"
docker logs fridgefusion-backend-mirror-ro1 2>&1 | grep "API Server.*8091"
docker logs fridgefusion-backend-mirror-ro2 2>&1 | grep "API Server.*8092"

# Проверить логи Caddy
docker compose logs caddy | grep health
```

---

## Итоговый статус

| Требование | Статус | Проверка |
|------------|--------|----------|
| Балансировка GET 2:1:1 | ✅ | `./test-load-balancing.sh` |
| POST/PUT/DELETE → master | ✅ | Логи backend-master |
| Обработка ошибок ro | ✅ | Роутинг на Caddy |
| PostgreSQL репликация | ✅ | `pg_stat_replication` |
| /mirror | ✅ | `curl /mirror/api/v2/health` |
| Server header | ✅ | `curl -I / \| grep Server` |
| Кеширование | ✅ | `curl -I /styles/` |
| gzip | ✅ | `curl -I \| grep gzip` |
| Grafana + Loki | ✅ | http://localhost:3000 |
| Bonus #1: HTTPS/HTTP2 | ✅ | `curl -I \| head -1` |
| Bonus #3: Health Checks | ✅ | Stop container test |

**Всего:** 9/9 основных требований + 2 бонуса = **100% выполнено**

---

## Остановка системы

```bash
# Остановить все сервисы
./stop.sh

# Или полностью удалить (включая volumes)
docker compose down -v
```

---

**Готово к демонстрации!** 🚀

Для полного руководства см. [LAB5-DEPLOYMENT.md](./LAB5-DEPLOYMENT.md)
