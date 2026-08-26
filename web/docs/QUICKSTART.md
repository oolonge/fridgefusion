# FridgeFusion Lab #5 - Quick Start

Быстрый старт для запуска и проверки Lab #5.

---

## Минимальные требования

- Docker 20.10+
- Docker Compose 1.29+
- 4GB RAM
- 10GB свободного места

---

## 1. Запуск (одна команда)

```bash
./start.sh
```

Первый запуск займет **10-15 минут** на сборку C++ бэкенда.

---

## 2. Проверка работы

### Основное приложение

```bash
# HTTP
curl http://localhost:8000/api/v2/health

# HTTPS (примите самоподписанный сертификат)
curl -k https://localhost:8443/api/v2/health
```

### Mirror

```bash
curl -k https://localhost:8443/mirror/api/v2/health
```

### Мониторинг

Откройте браузер: http://localhost:3000
- Логин: `admin`
- Пароль: `admin`

---

## 3. Тест балансировки

```bash
./test-load-balancing.sh
```

---

## 4. Просмотр логов

```bash
# Все сервисы
docker-compose logs -f

# Конкретный сервис
docker-compose logs -f caddy
docker-compose logs -f backend-master
```

---

## 5. Остановка

```bash
./stop.sh
```

---

## Endpoints

| Сервис | URL |
|--------|-----|
| Main App (HTTP) | http://localhost:8000 |
| Main App (HTTPS) | https://localhost:8443 |
| Mirror (HTTPS) | https://localhost:8443/mirror |
| API Docs | https://localhost:8443/api/docs |
| Grafana | http://localhost:3000 |
| PostgreSQL Master | localhost:5432 |
| PostgreSQL Slave | localhost:5433 |

---

## Проблемы?

См. полную документацию: [LAB5-DEPLOYMENT.md](./LAB5-DEPLOYMENT.md)

```bash
# Пересоздать всё с нуля
docker-compose down -v
./start.sh
```

---

## Что дальше?

1. ✅ Проверьте балансировку: `./test-load-balancing.sh`
2. ✅ Откройте Grafana и посмотрите логи
3. ✅ Проведите нагрузочное тестирование с ApacheBench
4. ✅ Заполните отчёт: [LOAD-TEST-REPORT.md](./LOAD-TEST-REPORT.md)

**Документация:**
- 📘 [LAB5-DEPLOYMENT.md](./LAB5-DEPLOYMENT.md) — полное руководство
- 📊 [LOAD-TEST-REPORT.md](./LOAD-TEST-REPORT.md) — шаблон отчёта
- 📝 [LAB5-SUMMARY.md](./LAB5-SUMMARY.md) — резюме реализации
