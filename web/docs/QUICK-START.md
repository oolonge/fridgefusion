# FridgeFusion Lab #5 - Быстрый старт

## 🚀 Первый запуск

```bash
# Убедитесь, что Docker запущен
docker info

# Запустите все сервисы
./start.sh
```

## 🔄 Перезапуск с очисткой данных

Если возникли проблемы с репликацией или нужно начать с чистого листа:

```bash
./restart-clean.sh
```

**Внимание:** Это удалит все данные и volumes!

## 📊 Проверка статуса

```bash
# Проверить статус всех контейнеров
docker compose ps

# Проверить статус репликации PostgreSQL
./check-replication.sh

# Проверить логи конкретного сервиса
docker compose logs -f postgres-master
docker compose logs -f postgres-slave
docker compose logs -f backend-master
docker compose logs -f caddy
```

## 🧪 Тестирование

```bash
# Проверить health endpoints
curl -k https://localhost:9443/api/v2/health
curl -k https://localhost:9443/mirror/api/v2/health

# Проверить балансировку нагрузки
./test-load-balancing.sh

# Открыть Grafana для мониторинга
open http://localhost:3000
# Логин: admin / admin
```

## 🛑 Остановка

```bash
# Остановить все сервисы (сохранить данные)
docker compose down

# Остановить и удалить volumes (полная очистка)
docker compose down -v
```

## 🔧 Порты

| Сервис | Порт | Описание |
|--------|------|----------|
| Caddy HTTP (Lab #5) | 9000 | HTTP load balancer |
| Caddy HTTPS (Lab #5) | 9443 | HTTPS load balancer |
| Caddy Local Dev | 8000 | Локальная разработка |
| Backend Master | 8080 | Master backend (прямой доступ) |
| Backend RO1 | 8081 | Read-only backend 1 |
| Backend RO2 | 8082 | Read-only backend 2 |
| Mirror Master | 8090 | Mirror master backend |
| Mirror RO1 | 8091 | Mirror read-only 1 |
| Mirror RO2 | 8092 | Mirror read-only 2 |
| PostgreSQL Master | 5434 | Master БД (R/W) |
| PostgreSQL Slave | 5435 | Slave БД (R/O replica) |
| Grafana | 3000 | Мониторинг |
| Loki | 3100 | Log aggregation |

## ⚠️ Важно

1. **Локальный PostgreSQL**: Локальный PostgreSQL работает на порту 5432 и не конфликтует с контейнерной БД (порты 5434/5435)
2. **Локальный Caddy**: Локальный Caddy для разработки работает на порту 8000, Lab #5 использует порты 9000/9443
3. **HTTPS сертификат**: Self-signed сертификат - нужно принять в браузере при первом обращении
4. **Первый запуск**: Может занять 10-15 минут на сборку C++ backend
5. **Переменные окружения**: Backend'ы используют `API_PORT` из docker-compose для определения портов

## 🐛 Устранение проблем

### postgres-slave не запускается

```bash
# Удалить volumes и начать заново
docker compose down -v
./start.sh
```

### Backend не может подключиться к БД

```bash
# Проверить логи БД
docker compose logs postgres-master
docker compose logs postgres-slave

# Проверить healthcheck
docker compose ps
```

### Caddy не балансирует нагрузку

```bash
# Проверить, что все backend-ы запущены
docker compose ps | grep backend

# Проверить Caddyfile синтаксис
docker exec fridgefusion-caddy caddy validate --config /etc/caddy/Caddyfile
```

## 📚 Дополнительная документация

- [LAB5-SUMMARY.md](LAB5-SUMMARY.md) - Полное описание реализации
- [LAB5-DEPLOYMENT.md](LAB5-DEPLOYMENT.md) - Детальное руководство по развёртыванию
- [LOAD-TEST-REPORT.md](LOAD-TEST-REPORT.md) - Шаблон отчёта по нагрузочному тестированию
