# Async Task Web Server

Учебный веб-сервер на C++20, Boost.Asio, Boost.Beast, OpenSSL и SQLite.

Проект разделён на две независимые статические библиотеки:

- `async_web_server` — HTTP/HTTPS транспорт, coroutine-сессии и статические файлы;
- `task_system` — модель задач, SQLite, REST API и планировщик таймеров.

Исполняемый файл связывает обе библиотеки через прикладной обработчик
`ApplicationRequestHandler`.

## Структура

```text
async_task_web_server/
├── CMakeLists.txt
├── app/
│   ├── ApplicationRequestHandler.cpp
│   ├── ApplicationRequestHandler.hpp
│   ├── TaskApiHandler.cpp
│   ├── TaskApiHandler.hpp
│   └── main.cpp
├── async_web_server/
│   ├── CMakeLists.txt
│   ├── include/server/
│   └── src/server/
├── task_system/
│   ├── CMakeLists.txt
│   ├── include/tasks/
│   └── src/tasks/
├── public/
│   ├── index.html
│   ├── styles.css
│   └── app.js
├── certs/
├── data/
└── scripts/
    └── generate-certificate.sh
```

## Зависимости

```bash
sudo apt update
sudo apt install -y \
    build-essential \
    cmake \
    libboost-system-dev \
    libssl-dev \
    libsqlite3-dev
```

## Сборка

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
```

В результате создаются:

```text
build/async_web_server/libasync_web_server.a
build/task_system/libtask_system.a
build/async_task_web_server
```

## Запуск HTTP

```bash
./build/async_task_web_server \
    --protocol http \
    --port 8080 \
    --public-dir build/public \
    --database build/data/tasks.db
```

Открыть в браузере:

```text
http://localhost:8080
```

## Запуск HTTPS

```bash
./scripts/generate-certificate.sh

./build/async_task_web_server \
    --protocol https \
    --port 8443 \
    --public-dir build/public \
    --database build/data/tasks.db \
    --cert certs/server.crt \
    --key certs/server.key
```

Открыть в браузере:

```text
https://localhost:8443
```

## Зависимости между целями

```text
async_task_web_server executable
├── async_web_server::async_web_server
└── task_system::task_system
```

`async_web_server` ничего не знает о задачах и SQLite. `task_system` ничего
не знает об HTTP, HTTPS и статических файлах. HTTP-адаптер `TaskApiHandler`
расположен в исполняемом приложении и связывает обе независимые библиотеки.

## Конфигурация через JSON/YAML

Параметры запуска можно передать через файл:

```bash
./build/async_task_web_server --config config.example.json
```

или:

```bash
./build/async_task_web_server --config config.example.yaml
```

CLI-аргументы имеют приоритет над файлом конфигурации:

```bash
./build/async_task_web_server --config config.example.yaml --port 9090
```

Поддерживаемые параметры:

```yaml
server:
  protocol: http
  host: 0.0.0.0
  port: 8080
  threads: 4
  public_dir: public

storage:
  database: data/tasks.db

tls:
  certificate_file: certs/server.crt
  private_key_file: certs/server.key

logging:
  level: info
```

## Логирование

Добавлен простой потокобезопасный логер без внешних зависимостей. Уровень
логирования задаётся через CLI или конфиг:

```bash
./build/async_task_web_server --log-level debug
```

Доступные уровни: `debug`, `info`, `warning`, `error`.

## Метрики

Метрики доступны в Prometheus text format по адресу:

```text
http://localhost:8080/metrics
```

Сейчас экспортируются счётчики HTTP-запросов, ответов по классам статусов,
необработанных исключений и событий планировщика задач.

## Unit-тесты TaskScheduler

Добавлены тесты для `TaskScheduler`:

- выполнение запланированной задачи;
- отмена задачи;
- повторное планирование и игнорирование старого callback таймера;
- восстановление задач из репозитория.

Запуск:

```bash
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build -j
ctest --test-dir build --output-on-failure
```
