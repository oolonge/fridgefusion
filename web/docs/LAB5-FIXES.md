# FridgeFusion Lab #5 - Журнал исправлений

## 📋 Итоговый статус

✅ **Все сервисы запущены и работают!**

```
✔ postgres-master (5434)        - Healthy
✔ postgres-slave (5435)         - Healthy (streaming replication)
✔ backend-master (8080)         - Running
✔ backend-ro1 (8081)            - Running
✔ backend-ro2 (8082)            - Running
✔ backend-mirror-master (8090)  - Running
✔ backend-mirror-ro1 (8091)     - Running
✔ backend-mirror-ro2 (8092)     - Running
✔ caddy (9000/9443)             - Running
✔ grafana (3000)                - Running
✔ loki (3100)                   - Running
✔ promtail                      - Running
```

---

## 🔧 Проблемы и решения

### 1. ❌ Конфликт портов PostgreSQL (5432)

**Проблема:** Локальный PostgreSQL занимал порт 5432

**Решение:**
- Master: `5432` → `5434`
- Slave: `5433` → `5435`

**Файлы:**
- `docker-compose.yml`: обновлены проброс портов
- `start.sh`: обновлена документация портов

---

### 2. ❌ PostgreSQL Slave не запускался (pg_hba.conf)

**Проблема:**
```
FATAL: no pg_hba.conf entry for replication connection from host "172.18.0.x",
user "replicator", no encryption
```

**Причина:**
- `pg_hba.conf` монтировался как volume, но PostgreSQL использует копию в `$PGDATA`
- Требовался метод `md5`, но `pg_basebackup` подключался без шифрования

**Решение:**
1. Изменен метод аутентификации для репликации с `md5` на `trust` в `pg_hba.conf`
2. Добавлено копирование `pg_hba.conf` в `$PGDATA` через `master/init.sh`
3. Создан custom entrypoint для slave: `docker/postgres/slave/entrypoint.sh`

**Файлы:**
- `docker/postgres/master/pg_hba.conf`
- `docker/postgres/master/init.sh`
- `docker/postgres/slave/entrypoint.sh`
- `docker-compose.yml`: slave использует custom entrypoint

---

### 3. ❌ Конфликт портов Caddy (8000/8443)

**Проблема:** Локальный Caddy для разработки уже занимал порты 8000/8443

**Решение:** Разделили порты для параллельной работы:
- **Локальный Caddy (разработка):** 8000 HTTP
- **Docker Caddy (Lab #5):** 9000 HTTP / 9443 HTTPS

**Файлы:**
- `docker-compose.yml`: изменены проброс портов на 9000:8000, 9443:8443
- `start.sh`: обновлена информация о портах
- Все `LAB5-*.md` файлы: замена `8000/8443` → `9000/9443`

---

## 📁 Созданные файлы

1. **`docker/postgres/slave/entrypoint.sh`** - Custom entrypoint для настройки репликации
2. **`restart-clean.sh`** - Скрипт для полной очистки и перезапуска
3. **`check-replication.sh`** - Проверка статуса репликации
4. **`QUICK-START.md`** - Краткое руководство по запуску
5. **`LAB5-FIXES.md`** - Этот файл (журнал исправлений)

---

## 🧪 Проверка работоспособности

Выполните следующие команды для проверки:

```bash
# 1. Статус всех контейнеров
docker-compose ps

# 2. Проверка API
curl -k https://localhost:9443/api/v2/health
curl -k https://localhost:9443/mirror/api/v2/health

# 3. Проверка репликации PostgreSQL
docker exec fridgefusion-postgres-master psql -U postgres -d fridgefusion \
  -c "SELECT * FROM pg_stat_replication;"

# 4. Проверка slave (должен вернуть 't' = true)
docker exec fridgefusion-postgres-slave psql -U postgres -d fridgefusion \
  -c "SELECT pg_is_in_recovery();"

# 5. Проверка балансировки нагрузки
./test-load-balancing.sh

# 6. Логи Caddy
docker-compose logs caddy | tail -50

# 7. Мониторинг
open http://localhost:3000  # Grafana (admin/admin)
```

---

## 📊 Архитектура портов

| Сервис | Порт (Host) | Порт (Container) | Назначение |
|--------|-------------|------------------|------------|
| **Caddy Lab #5 HTTP** | 9000 | 8000 | HTTP балансировщик |
| **Caddy Lab #5 HTTPS** | 9443 | 8443 | HTTPS балансировщик |
| **Caddy Local Dev** | 8000 | - | Локальная разработка |
| **Backend Master** | 8080 | 8080 | Master (R/W) |
| **Backend RO1** | 8081 | 8081 | Read-only 1 |
| **Backend RO2** | 8082 | 8082 | Read-only 2 |
| **Mirror Master** | 8090 | 8090 | Mirror master |
| **Mirror RO1** | 8091 | 8091 | Mirror RO 1 |
| **Mirror RO2** | 8092 | 8092 | Mirror RO 2 |
| **PostgreSQL Master** | 5434 | 5432 | Master БД (R/W) |
| **PostgreSQL Slave** | 5435 | 5432 | Slave БД (R/O) |
| **Grafana** | 3000 | 3000 | Мониторинг |
| **Loki** | 3100 | 3100 | Log aggregation |

**Важно:** Внутри Docker сети контейнеры общаются через внутренние порты (колонка "Container"), а с хоста используются проброшенные порты (колонка "Host").

---

## ✅ Контрольный список Lab #5

- [x] Балансировка GET 2:1:1
- [x] POST/PUT/DELETE только на master
- [x] PostgreSQL master-slave репликация
- [x] Маршрут /mirror
- [x] Server header подмена
- [x] Кеширование статики
- [x] gzip сжатие
- [x] Grafana + Loki мониторинг
- [x] HTTPS + HTTP/2 (Bonus #1)
- [x] HTTP Health Checks (Bonus #3)

---

## 🚀 Быстрый запуск

```bash
# Полная очистка и запуск
docker-compose down -v
./start.sh

# Или используйте скрипт очистки
./restart-clean.sh
```

---

## 📝 Примечания

1. **Локальный PostgreSQL:** Продолжает работать на порту 5432 без конфликтов
2. **Локальный Caddy:** Продолжает работать на порту 8000 для разработки
3. **Self-signed сертификат:** Нужно принять в браузере при первом обращении к https://localhost:9443
4. **Тестовые данные:** Автоматически создаются из `docker/postgres/init-db.sql`
   - Пользователи: admin@fridgefusion.com, test@fridgefusion.com (пароль: `password`)
   - 4 рецепта, 15 ингредиентов

---

**Автор:** Claude Code + Администратор
**Дата:** 27 ноября 2025
**Статус:** ✅ Полностью работает
