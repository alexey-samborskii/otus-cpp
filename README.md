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
