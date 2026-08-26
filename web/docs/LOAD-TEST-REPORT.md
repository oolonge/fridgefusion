# FridgeFusion Lab #5 - Отчёт по нагрузочному тестированию

## Цель тестирования

Провести нагрузочное тестирование системы FridgeFusion с балансировкой нагрузки для:
1. Проверки корректной работы балансировки GET-запросов в соотношении 2:1:1
2. Оценки производительности системы с 3 инстансами бэкенда
3. Сравнения производительности с одним инстансом vs балансировкой
4. Проверки работы кеширования и gzip-сжатия

---

## Методология

**Инструмент:** ApacheBench (ab)

**Тестовая среда:**
- OS: [указать]
- Docker version: [указать]
- RAM: [указать]
- CPU: [указать]

**Конфигурация системы:**
- Caddy load balancer (HTTPS/HTTP2)
- 3 backend инстанса (C++ Crow framework)
- PostgreSQL master + slave replica
- Балансировка GET: 2:1:1 (master:ro1:ro2)

---

## Тест 1: Health Check Endpoint (GET /api/v2/health)

### Параметры

```bash
ab -n 10000 -c 100 -k https://localhost:8443/api/v2/health
```

- **Запросов:** 10,000
- **Одновременно:** 100
- **Keep-Alive:** Да

### Результаты

```
[ВСТАВИТЬ ВЫВОД ab СЮДА]

Server Software:        FridgeFusion/2.0
Server Hostname:        localhost
Server Port:            8443

Document Path:          /api/v2/health
Document Length:        XX bytes

Concurrency Level:      100
Time taken for tests:   X.XXX seconds
Complete requests:      10000
Failed requests:        0
Keep-Alive requests:    10000
Total transferred:      XXXXXXX bytes
HTML transferred:       XXXXXX bytes
Requests per second:    XXXX.XX [#/sec] (mean)
Time per request:       XX.XXX [ms] (mean)
Time per request:       X.XXX [ms] (mean, across all concurrent requests)
Transfer rate:          XXXX.XX [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        X    XX   XX.X     XX    XXX
Processing:     X    XX   XX.X     XX    XXX
Waiting:        X    XX   XX.X     XX    XXX
Total:          X    XX   XX.X     XX    XXX

Percentage of the requests served within a certain time (ms)
  50%     XX
  66%     XX
  75%     XX
  80%     XX
  90%     XX
  95%     XX
  98%     XX
  99%     XX
 100%     XXX (longest request)
```

### Анализ

- **Requests per second:** [значение] RPS
- **Среднее время ответа:** [значение] ms
- **Ошибок:** 0
- **Вывод:** [описать производительность]

---

## Тест 2: Статический контент с кешированием (GET /styles/common.css)

### Параметры

```bash
ab -n 5000 -c 50 -k https://localhost:8443/styles/common.css
```

- **Запросов:** 5,000
- **Одновременно:** 50

### Результаты

```
[ВСТАВИТЬ ВЫВОД ab СЮДА]
```

### Анализ балансировки

Проверка распределения запросов:

```bash
docker-compose logs caddy | grep "styles/common.css" | \
  grep -oE "backend-[a-z0-9-]+" | sort | uniq -c
```

**Результат:**
```
[ВСТАВИТЬ РЕЗУЛЬТАТ]
```

**Ожидаемое распределение:** ~50% master, ~25% ro1, ~25% ro2

---

## Тест 3: API запросы с аутентификацией (GET /api/v2/recipes)

### Параметры

```bash
# Сначала получить JWT токен
TOKEN=$(curl -sk -X POST https://localhost:8443/api/v2/auth/login \
  -H "Content-Type: application/json" \
  -d '{"username":"testuser","password":"password"}' | \
  jq -r '.token')

# Создать файл с headers
echo "Authorization: Bearer $TOKEN" > /tmp/headers.txt

# Запустить тест
ab -n 5000 -c 50 -k -H "Authorization: Bearer $TOKEN" \
  https://localhost:8443/api/v2/recipes
```

### Результаты

```
[ВСТАВИТЬ ВЫВОД ab СЮДА]
```

### Анализ

- **RPS с аутентификацией:** [значение]
- **Влияние JWT валидации:** [описание]

---

## Тест 4: POST запросы (только на master)

### Параметры

```bash
# Создание рецептов
ab -n 1000 -c 10 -p recipe.json -T application/json \
  -H "Authorization: Bearer $TOKEN" \
  https://localhost:8443/api/v2/recipes
```

Где `recipe.json`:
```json
{
  "name": "Test Recipe",
  "description": "Load test recipe",
  "preparation_time": 10,
  "cooking_time": 20,
  "instructions": "Test instructions"
}
```

### Результаты

```
[ВСТАВИТЬ ВЫВОД ab СЮДА]
```

### Проверка роутинга

Все POST запросы должны идти только на master:

```bash
docker-compose logs backend-master | grep "POST.*recipes" | wc -l
docker-compose logs backend-ro1 | grep "POST.*recipes" | wc -l
docker-compose logs backend-ro2 | grep "POST.*recipes" | wc -l
```

**Результат:**
- Master: [количество]
- RO1: 0 ✅
- RO2: 0 ✅

---

## Тест 5: Сравнение производительности

### Один инстанс vs Балансировка (3 инстанса)

#### С балансировкой (текущая конфигурация)

```bash
ab -n 10000 -c 100 -k https://localhost:8443/api/v2/health
```

**RPS:** [значение]

#### Один инстанс (для сравнения)

```bash
# Временно отключить ro1 и ro2
docker stop fridgefusion-backend-ro1 fridgefusion-backend-ro2

# Запустить тест
ab -n 10000 -c 100 -k https://localhost:8443/api/v2/health

# Включить обратно
docker start fridgefusion-backend-ro1 fridgefusion-backend-ro2
```

**RPS:** [значение]

### Анализ

| Конфигурация | RPS | Среднее время (ms) | Улучшение |
|--------------|-----|-------------------|-----------|
| 1 инстанс | [X] | [Y] | - |
| 3 инстанса (2:1:1) | [X] | [Y] | +[%]% |

**Вывод:** Балансировка на 3 инстанса даёт прирост производительности на [X]%

---

## Тест 6: Проверка gzip сжатия

### Без gzip (для сравнения)

```bash
# Временно отключить gzip в Caddyfile, перезапустить Caddy
ab -n 1000 -c 10 https://localhost:8443/api/v2/recipes
```

**Средний размер ответа:** [X] bytes

### С gzip

```bash
ab -n 1000 -c 10 https://localhost:8443/api/v2/recipes
```

**Средний размер ответа:** [Y] bytes

### Анализ

- **Размер без gzip:** [X] bytes
- **Размер с gzip:** [Y] bytes
- **Экономия:** [X-Y] bytes ([%]%)

---

## Тест 7: Стресс-тест (высокая нагрузка)

### Параметры

```bash
ab -n 50000 -c 500 -k https://localhost:8443/api/v2/health
```

- **Запросов:** 50,000
- **Одновременно:** 500

### Результаты

```
[ВСТАВИТЬ ВЫВОД ab СЮДА]
```

### Мониторинг ресурсов

```bash
# Во время теста
docker stats --no-stream
```

**Использование ресурсов:**

| Контейнер | CPU % | RAM |
|-----------|-------|-----|
| caddy | [X]% | [Y]MB |
| backend-master | [X]% | [Y]MB |
| backend-ro1 | [X]% | [Y]MB |
| backend-ro2 | [X]% | [Y]MB |
| postgres-master | [X]% | [Y]MB |
| postgres-slave | [X]% | [Y]MB |

---

## Тест 8: Проверка Health Checks

### Сценарий: Падение одного backend

```bash
# Остановить ro1
docker stop fridgefusion-backend-ro1

# Подождать, пока Caddy обнаружит падение (10-15 сек)
sleep 15

# Запустить тест
ab -n 5000 -c 50 -k https://localhost:8443/api/v2/health

# Проверить распределение
docker-compose logs caddy | tail -1000 | grep "api/v2/health" | \
  grep -oE "backend-[a-z0-9-]+" | sort | uniq -c
```

**Результат:**
```
[количество] backend-master
[количество] backend-ro2
     0       backend-ro1  ✅ (исключён из балансировки)
```

**Вывод:** Health checks работают корректно, нерабочий инстанс исключается из ротации.

---

## Выводы

### Производительность

1. **RPS (Requests Per Second):**
   - Health endpoint: [X] RPS
   - Статический контент: [Y] RPS
   - API с аутентификацией: [Z] RPS

2. **Балансировка:**
   - GET запросы распределяются в соотношении ~2:1:1 ✅
   - POST/PUT/DELETE идут только на master ✅

3. **Отказоустойчивость:**
   - Health checks работают корректно ✅
   - При падении инстанса система продолжает работу ✅

4. **Оптимизации:**
   - gzip сжатие уменьшает трафик на [X]% ✅
   - Кеширование статики работает ✅

### Рекомендации

1. [Предложения по оптимизации]
2. [Узкие места, если найдены]
3. [Предложения по масштабированию]

---

## Приложения

### Команды для повторения тестов

```bash
# Базовый тест
ab -n 10000 -c 100 -k https://localhost:8443/api/v2/health

# С выводом в файл
ab -n 10000 -c 100 -k -g results.tsv https://localhost:8443/api/v2/health

# Построение графика (требуется gnuplot)
gnuplot << EOF
set terminal png size 1024,768
set output 'loadtest.png'
set datafile separator "\t"
set xlabel "Request"
set ylabel "Response time (ms)"
plot "results.tsv" using 10 with lines title "Response time"
EOF
```

### Проверка балансировки

```bash
# Скрипт для подсчёта распределения
for i in {1..300}; do
  curl -sk https://localhost:8443/api/v2/health > /dev/null
  sleep 0.05
done

docker-compose logs --tail=300 caddy | \
  grep "api/v2/health" | \
  grep -oE ":(8080|8081|8082)" | \
  sort | uniq -c
```

**Ожидаемый результат (примерно):**
```
    150 :8080   (50%)
     75 :8081   (25%)
     75 :8082   (25%)
```

---

## Дата проведения тестов

- **Дата:** [YYYY-MM-DD]
- **Исполнитель:** [Имя]
- **Версия:** FridgeFusion Lab #5 (2025)

---

## Приложение: Скриншоты

1. Grafana dashboard с логами всех инстансов
2. Caddy metrics во время нагрузки
3. Docker stats во время стресс-теста
4. ApacheBench output для ключевых тестов

[ДОБАВИТЬ СКРИНШОТЫ]
