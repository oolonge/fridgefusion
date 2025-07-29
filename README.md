# FridgeFusion

**Десктопное приложение для поиска рецептов с системой заменителей ингредиентов**

FridgeFusion — система управления рецептами с поиском по ингредиентам. Производит автоматический подбор заменителей ингредиентов на основе коэффициентов похожести.

## Функциональные возможности

- Поиск рецептов по ингредиентам с учетом возможных замен
- Многопользовательская система с ролевой моделью (гость, пользователь, администратор)
- Система избранных рецептов и отзывов с рейтингами
- Административная панель для управления пользователями и контентом
- База знаний из 250 ингредиентов с коэффициентами похожести

## Архитектура

Проект построен на **Clean Architecture** с четким разделением слоев:

```
├── Domain Layer      # Business entities & DTOs
├── Application Layer # Use cases & business logic  
├── Infrastructure    # Database & external services
└── Presentation      # Qt UI (MVP pattern)
```

## Технологический стек

- Язык: C++17
- GUI: Qt 6 (Widgets)
- База данных: PostgreSQL
- Архитектура: Clean Architecture + MVP
- Безопасность: PBKDF2 хеширование паролей
- Тестирование: Многопоточное стресс-тестирование

## Системные требования

- Qt 6.0+ с SQL модулем
- PostgreSQL 12+
- C++17 совместимый компилятор
- Поддерживаемые ОС: Windows, macOS, Linux

## Установка и запуск

### 1. Клонирование репозитория
```bash
git clone https://github.com/username/FridgeFusion.git
cd FridgeFusion
```

### 2. Настройка базы данных
```bash
# Создайте БД PostgreSQL и выполните:
psql -d your_db -f struct_script.sql
psql -d your_db -f trigger.sql
```

### 3. Конфигурация
Создайте `config.json` в корне проекта:
```json
{
  "database": {
    "host": "localhost",
    "port": 5432,
    "name": "your_db_name",
    "username": "your_username",
    "password": "your_password"
  }
}
```

### 4. Сборка
```bash
qmake FridgeFusion.pro
make
./bin/FridgeFusion
```

## Тестирование

Проект включает многопоточное стресс-тестирование:

```bash
cd stress_tests
qmake stress_tests.pro
make
./stress_tests
```

Система тестирует производительность при нагрузке до 100 одновременных пользователей с четырьмя сценариями использования: регистрация/аутентификация, поиск рецептов, управление избранным, система отзывов.
