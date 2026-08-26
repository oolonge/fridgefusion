# Автостарт Caddy (Bonus #3)

### Проверить статус

```bash
launchctl list | grep fridgefusion
```

Вывод должен быть:
```
12345   0   com.fridgefusion.caddy
```

Где:
- `12345` — PID процесса
- `0` — статус (0 = работает, -1 = ошибка)

### Логи

```bash
# Основной лог
tail -f /tmp/caddy.log

# Ошибки
tail -f /tmp/caddy-error.log

# Логи доступа
tail -f /tmp/caddy_access.log
```

Должно работать: http://localhost:8000

## Управление сервисом

### Остановить

```bash
launchctl unload ~/Library/LaunchAgents/com.fridgefusion.caddy.plist
```

### Запустить снова

```bash
launchctl load ~/Library/LaunchAgents/com.fridgefusion.caddy.plist
```

### Перезапустить

```bash
launchctl unload ~/Library/LaunchAgents/com.fridgefusion.caddy.plist
launchctl load ~/Library/LaunchAgents/com.fridgefusion.caddy.plist
```

### Удалить из автостарта

```bash
launchctl unload ~/Library/LaunchAgents/com.fridgefusion.caddy.plist
rm ~/Library/LaunchAgents/com.fridgefusion.caddy.plist
```

---

## Доказательство использования всех ядер

### Способ 1 — Логи Caddy

При запуске Caddy выводит:
```
GOMAXPROCS=8
```

Это означает использование всех 8 ядер.

### Способ 2 — Activity Monitor

1. Открой **Activity Monitor** (Мониторинг системы)
2. Найди процесс `caddy`
3. Посмотри столбец **Threads** — должно быть ≥8
4. Колонка **% CPU** может показывать >100% (означает использование нескольких ядер)

### Способ 3 — Команда top

```bash
top -pid $(pgrep caddy)
```

Смотри:
- `TH` (Threads) — количество потоков
- `%CPU` — может превышать 100%

### Способ 4 — Страница /status/

Открой: http://localhost:8000/status/

Там видны метрики:
- **Goroutines** — количество конкурентных потоков Go
- **Память** — использование RAM
- **HTTP запросов** — общее количество обработанных запросов

---

## Файлы автостарта

- **plist:** `~/Library/LaunchAgents/com.fridgefusion.caddy.plist`
- **Логи:** `/tmp/caddy.log`, `/tmp/caddy-error.log`
- **Конфиг:** `/Users/administrator/Desktop/Web/FridgeFusion/web_server/Caddyfile`
