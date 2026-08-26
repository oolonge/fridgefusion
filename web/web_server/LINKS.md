# FridgeFusion - Навигация по сервисам

## Основные страницы

| URL | Описание |
|-----|----------|
| http://localhost:8000 | Главная страница (будущее SPA) |
| http://localhost:8000/management/ | Панель управления со всеми ссылками |
| http://localhost:8000/legacy/ | Ссылки на скачивание CLI и Desktop версий |
| http://localhost:8000/reserved/ | Зарезервированный путь (дублирует главную) |

## API и документация

| URL | Описание |
|-----|----------|
| http://localhost:8000/api/v2/health | Health check API |
| http://localhost:8000/api/docs | Swagger UI документация |
| http://localhost:8000/documentation/ | README документация проекта |

## Администрирование

| URL | Описание |
|-----|----------|
| http://localhost:8000/admin | Adminer - веб-интерфейс для PostgreSQL |
| http://localhost:8000/status/ | Статус и метрики сервера |
| http://localhost:8000/metrics | Raw метрики Caddy (Prometheus формат) |

## Прямой доступ к API (без Caddy)

| URL | Описание |
|-----|----------|
| http://localhost:8080/api/v2/health | API Health check |
| http://localhost:8080/api/docs | Swagger UI |
| http://localhost:8080/api/v2/recipes | Список рецептов |
| http://localhost:8080/api/v2/ingredients | Список ингредиентов |
| http://localhost:8080/api/v2/categories | Список категорий |

## Порты сервисов

- **8000** — Caddy (web-сервер, маршрутизация)
- **8080** — FridgeFusion REST API (Crow)
- **8081** — Adminer (PHP)
- **5432** — PostgreSQL

## Adminer - подключение к БД

Для входа в Adminer используй:
- **System:** PostgreSQL
- **Server:** localhost
- **Username:** postgres
- **Password:** (пароль от PostgreSQL)
- **Database:** postgres (или fridgefusion)


Term 1:
~/Desktop/Web/FridgeFusion/web_api/build/fridgefusion_api

Term 2:
cd /Users/administrator/Desktop/Web/FridgeFusion/web_server/adminer
php -S localhost:8081 router.php

Term 3:
pkill caddy; sleep 1; cd /Users/administrator/Desktop/Web/FridgeFusion/web_server && caddy run --config Caddyfile --adapter caddyfile
