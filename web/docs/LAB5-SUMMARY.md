# FridgeFusion Lab #5 - Краткое резюме реализации

## Статус: ✅ Полностью реализовано

**Основные требования:** 7/7 ✅
**Бонусы:** 2/5 ✅ (Bonus #1, Bonus #3)

---

## Реализованные требования

### 1. ✅ Балансировка GET-запросов (2:1:1)

**Требование:** Запустить 2 дополнительных инстанса бэкенда с правами read-only, настроить балансировку GET в соотношении 2:1:1.

**Реализация:**
- 3 backend инстанса: master (8080), ro1 (8081), ro2 (8082)
- Caddy load balancer с weighted balancing
- Master получает вес 2, ro1 и ro2 — по 1
- Read-only инстансы подключены к slave PostgreSQL с пользователем `readonly_user`

**Файлы:**
- `docker-compose.yml`: определение 3 backend services
- `docker/caddy/Caddyfile`: балансировка через `reverse_proxy` с multiple upstreams
- `docker/backend/config-master.json`, `config-ro1.json`, `config-ro2.json`

**Проверка:**
```bash
./test-load-balancing.sh
docker-compose logs caddy | grep -E "8080|8081|8082"
```

---

### 2. ✅ POST/PUT/PATCH/DELETE только на master

**Требование:** Запросы на запись должны идти только на инстанс с правами на запись.

**Реализация:**
- Caddy маршрутизирует по HTTP методу через `@api_write` matcher
- POST/PUT/PATCH/DELETE → только `backend-master:8080`
- Обработка на уровне reverse proxy (read-only инстансы вообще не получают write запросы)

**Файлы:**
- `docker/caddy/Caddyfile`:
  ```caddy
  @api_write {
    path /api/v2/*
    method POST PUT PATCH DELETE
  }
  handle @api_write {
    reverse_proxy backend-master:8080
  }
  ```

**Проверка:**
```bash
curl -X POST https://localhost:9443/api/v2/auth/register -d {...}
docker-compose logs backend-master | grep POST  # есть
docker-compose logs backend-ro1 | grep POST     # нет
```

---

### 3. ✅ Обработка ошибок при записи в read-only

**Требование:** Обрабатывать ошибки при попытке обратиться на запись к инстансам на чтение.

**Реализация:**
- Ошибки обрабатываются **до того, как дойдут до бэкенда** — на уровне Caddy
- Write-запросы вообще не маршрутизируются на read-only инстансы
- Если бы запрос дошёл до ro-бэкенда, PostgreSQL вернул бы `ERROR: permission denied` → бэкенд вернул бы HTTP 500

**Дополнительно:**
- Read-only инстансы используют `readonly_user` в PostgreSQL
- `readonly_user` имеет только `SELECT` права

**Проверка:**
```bash
# Прямой запрос к ro-инстансу (минуя балансировщик) должен упасть
curl -X POST http://localhost:8081/api/v2/recipes -d {...}
# Вернёт ошибку доступа к БД
```

---

### 4. ✅ PostgreSQL master-slave репликация

**Требование:** Настроить репликацию БД (master-slave).

**Реализация:**
- **Master** (5432): full read-write, включен WAL streaming
- **Slave** (5433): read-only replica, hot standby mode
- Репликация через `pg_basebackup` + streaming replication
- Пользователь `replicator` с правами REPLICATION

**Файлы:**
- `docker/postgres/master/postgresql.conf`: `wal_level=replica`, `max_wal_senders=3`
- `docker/postgres/slave/init.sh`: `pg_basebackup` + создание `standby.signal`
- `docker/postgres/init-db.sql`: схема и тестовые данные

**Проверка:**
```bash
# На master: проверить активную репликацию
docker exec -it fridgefusion-postgres-master psql -U postgres \
  -c "SELECT * FROM pg_stat_replication;"

# На slave: проверить recovery mode
docker exec -it fridgefusion-postgres-slave psql -U postgres \
  -c "SELECT pg_is_in_recovery();"  # должно вернуть true
```

---

### 5. ✅ Маршрут /mirror

**Требование:** Настроить `/mirror` с отдельно развёрнутой версией приложения, все относительные URL работают с префиксом `/mirror`.

**Реализация:**
- Дополнительные 3 backend инстанса: mirror-master (8090), mirror-ro1 (8091), mirror-ro2 (8092)
- Caddy использует `uri strip_prefix /mirror` для корректной маршрутизации
- Балансировка аналогично main: 2:1:1
- БД используется та же (master/slave)

**Файлы:**
- `docker-compose.yml`: services `backend-mirror-master`, `backend-mirror-ro1`, `backend-mirror-ro2`
- `docker/caddy/Caddyfile`: секция `handle /mirror/*` с балансировкой

**Проверка:**
```bash
curl https://localhost:9443/mirror/api/v2/health
curl https://localhost:9443/mirror/
```

**Итого:** 6 backend инстансов (3 main + 3 mirror)

---

### 6. ✅ Подмена Server header, кеширование, gzip

**Требование:**
- Подменять Server header на название приложения
- Настроить кеширование GET (кроме /api)
- Настроить gzip

**Реализация:**

**Server header:**
```caddy
header Server "FridgeFusion/2.0 (Lab#5)"
```

**Кеширование:**
```caddy
@static {
  path /styles/* /legacy/* /documentation/* ...
}
handle @static {
  header Cache-Control "public, max-age=3600"
  ...
}
```

**gzip:**
```caddy
encode gzip
```

**Проверка:**
```bash
curl -I https://localhost:9443/ | grep "Server"
# Server: FridgeFusion/2.0 (Lab#5)

curl -I https://localhost:9443/styles/common.css | grep "Cache-Control"
# Cache-Control: public, max-age=3600

curl -I https://localhost:9443/api/v2/health | grep "Content-Encoding"
# Content-Encoding: gzip
```

---

### 7. ✅ Grafana + Loki мониторинг

**Требование:** Настроить Grafana + Loki для визуализации логов со всех 2×3=6 инстансов.

**Реализация:**
- **Loki** (3100): log aggregation
- **Promtail**: собирает логи из 6 backend контейнеров + Caddy
- **Grafana** (3000): дашборды для визуализации

**Файлы:**
- `docker/loki/loki-config.yml`
- `docker/promtail/promtail-config.yml`: 7 job-ов (6 backends + caddy)
- `docker/grafana/datasources.yml`: автоконфигурация Loki

**Проверка:**
```bash
# Открыть Grafana
http://localhost:3000  # admin/admin

# В Explore выполнить:
{job=~"backend.*"}

# Должны быть видны логи со всех 6 инстансов
```

---

## Бонусы

### ✅ Bonus #1: HTTPS + HTTP/2

**Требование:** HTTPS с самоподписанным сертификатом + HTTP/2.

**Реализация:**
- Caddy автоматически генерирует self-signed сертификат через `tls internal`
- HTTP/2 включен по умолчанию в Caddy
- Порт 8443 для HTTPS

**Файлы:**
- `docker/caddy/Caddyfile`:
  ```caddy
  :8443 {
    tls internal
    ...
  }
  ```

**Проверка:**
```bash
curl -Ik https://localhost:9443/ | head -1
# HTTP/2 200
```

**Note:** ServerPush/Preload не реализован (не критично для минимальных требований).

---

### ✅ Bonus #3: HTTP Health Checks

**Требование:** Настроить HTTP Health Checks.

**Реализация:**
- Caddy выполняет health checks каждые 10 секунд на endpoint `/api/v2/health`
- При 3 неудачных попытках подряд инстанс исключается из балансировки на 30 секунд
- Автоматическое восстановление при появлении успешных ответов

**Файлы:**
- `docker/caddy/Caddyfile`:
  ```caddy
  reverse_proxy {
    health_uri /api/v2/health
    health_interval 10s
    health_timeout 5s
    health_status 2xx
    fail_duration 30s
    max_fails 3
  }
  ```

**Проверка:**
```bash
# Остановить ro1
docker stop fridgefusion-backend-ro1
sleep 15

# Запросы теперь только на master и ro2
./test-load-balancing.sh

# Запустить ro1 обратно — автоматически вернётся в балансировку
docker start fridgefusion-backend-ro1
```

---

## Не реализованные бонусы (опционально)

### ❌ Bonus #2: HTTP/3

**Причина:** Экспериментальная функция, нестабильная поддержка в Caddy и браузерах.

### ❌ Bonus #4: Кластер PostgreSQL

**Причина:** Master-master репликация в PostgreSQL требует сложной настройки (Patroni/Stolon). Для демонстрации достаточно master-slave.

### ❌ Bonus #5: Service Discovery (Consul)

**Причина:** Дополнительная сложность. Для статической конфигурации не требуется.

---

## Архитектурные решения

### Почему Caddy, а не nginx?

1. **Caddy проще в конфигурации** для weighted load balancing
2. **HTTPS из коробки** — автоматическая генерация self-signed сертификатов
3. **HTTP/2 по умолчанию**
4. **Health checks встроены** в reverse_proxy директиву
5. В задании сказано "пример с nginx", но можно использовать альтернативы

### Почему multi-stage Docker build?

- Сборка C++ бэкенда требует Qt6, CMake, Boost → тяжёлый builder image
- Runtime image содержит только бинарник + shared libraries → компактный (~200MB vs ~2GB)

### Почему 6 backend инстансов?

- Требование: 3 инстанса для main app + /mirror с аналогичной балансировкой
- /mirror должен быть "полноценной версией приложения"
- Итого: 3 main + 3 mirror = 6 инстансов

---

## Структура итоговой конфигурации

```
Services (11 containers):
├── postgres-master (R/W)
├── postgres-slave (R/O replica)
├── backend-master (8080) ─┐
├── backend-ro1 (8081)      ├─ Main app
├── backend-ro2 (8082) ─────┘
├── backend-mirror-master (8090) ─┐
├── backend-mirror-ro1 (8091)      ├─ Mirror
├── backend-mirror-ro2 (8092) ─────┘
├── caddy (8000 HTTP, 8443 HTTPS)
├── loki + promtail
└── grafana
```

---

## Как запустить

```bash
# Дать права на выполнение
chmod +x start.sh stop.sh test-load-balancing.sh

# Запустить всё
./start.sh

# Проверить работу
curl -k https://localhost:9443/api/v2/health
curl -k https://localhost:9443/mirror/api/v2/health

# Проверить балансировку
./test-load-balancing.sh

# Открыть мониторинг
open http://localhost:3000

# Остановить
./stop.sh
```

**Первый запуск:** 10-15 минут на сборку C++ бэкенда.

---

## Документация

- **LAB5-DEPLOYMENT.md** — полное руководство по развёртыванию
- **LOAD-TEST-REPORT.md** — шаблон для отчёта по нагрузочному тестированию
- **start.sh** — скрипт запуска
- **test-load-balancing.sh** — проверка балансировки

---

## Соответствие требованиям

| Требование | Статус | Файл/Раздел |
|------------|--------|-------------|
| Балансировка GET 2:1:1 | ✅ | Caddyfile:26-47 |
| POST/PUT/DELETE → master | ✅ | Caddyfile:49-63 |
| Обработка ошибок ro-инстансов | ✅ | Маршрутизация на уровне Caddy |
| PostgreSQL репликация | ✅ | docker/postgres/master+slave |
| /mirror | ✅ | Caddyfile:125-210, 3 mirror backends |
| Server header | ✅ | Caddyfile:24 |
| Кеширование | ✅ | Caddyfile:71-77 |
| gzip | ✅ | Caddyfile:22 |
| Grafana + Loki | ✅ | docker/loki + promtail + grafana |
| Bonus #1: HTTPS/HTTP2 | ✅ | Caddyfile:8,19 |
| Bonus #3: Health Checks | ✅ | Caddyfile:36-41 |

---

## Выводы

Реализованы **все основные требования** Lab #5 + **2 бонуса** (Bonus #1, #3).

Система полностью функциональна:
- ✅ Балансировка нагрузки с контролем здоровья инстансов
- ✅ Репликация БД для горизонтального масштабирования чтения
- ✅ Разделение read/write операций на архитектурном уровне
- ✅ HTTPS + HTTP/2 для безопасности и производительности
- ✅ Централизованный мониторинг логов со всех компонентов
- ✅ Кеширование и сжатие для оптимизации трафика

**Готово к демонстрации и нагрузочному тестированию.**

---

Автор: [Ваше имя]
Дата: 2025
Lab: #5 — WEB SERVER Senior DevOps
