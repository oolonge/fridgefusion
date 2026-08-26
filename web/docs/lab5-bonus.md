# Lab #5 - Bonus Tasks

## Реализованные бонусы

| Бонус | Статус | Описание |
|-------|--------|----------|
| #1 | Выполнен | HTTP/2 (h2c) |
| #3 | Выполнен | HTTP Health Checks |

---

## Bonus #1: HTTP/2

**Реализация:** HTTP/2 over cleartext (h2c) — HTTP/2 без TLS.

**Конфигурация** ([Caddyfile.lab5](../docker/caddy/Caddyfile.lab5)):
```caddy
{
    servers {
        protocols h1 h2c
    }
}
```

### Демонстрация

```bash
# Проверка HTTP/2 (флаг --http2-prior-knowledge для прямого h2c)
curl --http2-prior-knowledge -v http://localhost:9000/health 2>&1 | grep "HTTP/2"
```

**Ожидаемый вывод:**
```
> GET /health HTTP/2
< HTTP/2 200
```

### Что такое HTTP/2?

HTTP/2 — бинарный протокол с:
- **Мультиплексированием** — несколько запросов по одному соединению
- **Сжатием заголовков** (HPACK)
- **Server Push** — сервер отправляет ресурсы до запроса клиента
- **Приоритизацией потоков**

h2c (HTTP/2 cleartext) — HTTP/2 без TLS, используется для внутренних сервисов.

---

## Bonus #3: HTTP Health Checks

**Реализация:** Caddy автоматически проверяет доступность backend-серверов.

**Конфигурация** ([Caddyfile.lab5](../docker/caddy/Caddyfile.lab5)):
```caddy
reverse_proxy backend-master:8080 backend-ro1:8081 backend-ro2:8082 {
    health_uri /api/v2/health
    health_interval 10s
    health_timeout 5s
    health_status 2xx
    fail_duration 30s
    max_fails 3
}
```

### Демонстрация

```bash
# 1. Остановить один backend
docker stop fridgefusion-backend-ro1

# 2. Отправить запросы (ro1 исключён из балансировки)
for i in {1..10}; do curl -s http://localhost:9000/api/v2/health > /dev/null; done

# 3. Проверить распределение (ro1 отсутствует)
docker compose -f docker-compose.lab5.yml logs backend-master --tail=20 | grep "GET /api/v2/health" | wc -l
docker compose -f docker-compose.lab5.yml logs backend-ro2 --tail=20 | grep "GET /api/v2/health" | wc -l

# 4. Восстановить backend
docker start fridgefusion-backend-ro1
```

**Результат:** После остановки ro1 запросы идут только на master и ro2. После восстановления ro1 автоматически возвращается в балансировку.

---

## Не реализованные бонусы

| Бонус | Причина |
|-------|---------|
| #2 HTTP/3 | Экспериментальная функция, нестабильная поддержка |
| #4 PostgreSQL Cluster | Требует Patroni/Stolon, избыточно для демо |
| #5 Service Discovery | Consul добавляет сложность без практической пользы для лабы |
