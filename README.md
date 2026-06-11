# Async Web Server

Асинхронный HTTP/HTTPS-сервер на C++20, реализованный на базе Boost.Asio, Boost.Beast и C++20 coroutines.

Сервер поддерживает два режима запуска:

- `http` — обычный HTTP поверх TCP;
- `https` — HTTP поверх TLS/SSL.

Режим работы выбирается параметром командной строки.

---

## Возможности

- Асинхронная обработка клиентских подключений.
- Использование C++20 coroutines через `co_await`.
- Отдельная coroutine-сессия на каждое клиентское подключение.
- Поддержка HTTP и HTTPS.
- Раздача статических файлов из директории `public`.
- Обработка `SIGINT` и `SIGTERM` для корректной остановки сервера.
- Настраиваемые параметры запуска:
  - протокол;
  - host;
  - port;
  - количество worker-потоков;
  - директория со статическими файлами;
  - путь к TLS-сертификату и private key.

---

## Требования

Для сборки проекта необходимы:

- C++20 совместимый компилятор;
- CMake;
- Boost;
- OpenSSL;
- Threads.

Пример проверенного окружения:

```bash
g++ --version
cmake --version
```

---

## Структура проекта

```text
.
├── async_web_server
│   ├── app
│   │   └── main.cpp
│   ├── CMakeLists.txt
│   ├── include
│   │   └── server
│   │       ├── HttpRequestHandler.hpp
│   │       ├── HttpRequest.hpp
│   │       ├── HttpResponse.hpp
│   │       ├── HttpSession.hpp
│   │       ├── HttpsSession.hpp
│   │       ├── HttpsWebServer.hpp
│   │       ├── HttpWebServer.hpp
│   │       ├── Router.hpp
│   │       ├── SslContext.hpp
│   │       ├── StaticFileHandler.hpp
│   │       └── WebServer.hpp
│   ├── public
│   │   ├── index.html
│   │   └── static
│   │       └── style.css
│   └── src
│       └── server
│           ├── HttpRequest.cpp
│           ├── HttpRequestHandler.cpp
│           ├── HttpResponse.cpp
│           ├── HttpSession.cpp
│           ├── HttpsSession.cpp
│           ├── HttpsWebServer.cpp
│           ├── HttpWebServer.cpp
│           ├── Router.cpp
│           ├── SslContext.cpp
│           └── StaticFileHandler.cpp
├── certs
│   ├── server.crt
│   └── server.key
├── CMakeLists.txt
├── doc
│   └── async_web_server_structure.drawio
├── Doxyfile
├── README.md
└── tests
    ├── CMakeLists.txt
    ├── HttpRequestHandlerTests.cpp
    ├── HttpResponseTests.cpp
    └── RouterTests.cpp
```

---

## Архитектура

Общая схема работы сервера:

```text
main.cpp
    |
    |-- parseArguments()
    |
    |-- --protocol http
    |       |
    |       └── HttpWebServer
    |               |
    |               └── HttpSession
    |                       |
    |                       └── tcp::socket
    |
    |-- --protocol https
            |
            └── HttpsWebServer
                    |
                    └── HttpsSession
                            |
                            └── ssl::stream<tcp::socket>
```

Обработка HTTP-запроса выполняется по следующей цепочке:

```text
Boost.Beast request
        |
        v
server::HttpRequest
        |
        v
HttpRequestHandler
        |
        v
Router
        |
        v
StaticFileHandler
        |
        v
server::HttpResponse
        |
        v
Boost.Beast response
```

---

## Сборка

Из корня проекта:

```bash
rm -rf build
cmake -S . -B build
cmake --build build --parallel
```

Или одной командой:

```bash
rm -rf build && cmake -S . -B build && cmake --build build --parallel
```

После успешной сборки должен появиться исполняемый файл:

```text
build/async_web_server/async_web_server
```

---

## Параметры запуска

Сервер поддерживает следующие параметры:

```text
--protocol http|https
--host <address>
--port <port>
--threads <count>
--public <path>
--cert <path>
--key <path>
```

Также поддерживаются короткие варианты выбора протокола:

```text
--http
--https
```

---

## Запуск HTTP-сервера

Из корня проекта:

```bash
./build/async_web_server/async_web_server \
    --protocol http \
    --port 8080 \
    --public async_web_server/public
```

Открыть в браузере:

```text
http://localhost:8080/
```

Проверить через `curl`:

```bash
curl http://localhost:8080/
```

---

## Запуск HTTPS-сервера

Для HTTPS нужны сертификат и private key.

Пример запуска:

```bash
./build/async_web_server/async_web_server \
    --protocol https \
    --port 8443 \
    --public async_web_server/public \
    --cert certs/server.crt \
    --key certs/server.key
```

Открыть в браузере:

```text
https://localhost:8443/
```

Проверить через `curl`:

```bash
curl -k https://localhost:8443/
```

Параметр `-k` нужен, если используется самоподписанный сертификат.

---

## Создание self-signed сертификата

Если сертификатов ещё нет, их можно создать командой:

```bash
mkdir -p certs

openssl req -x509 -newkey rsa:2048 -nodes \
    -keyout certs/server.key \
    -out certs/server.crt \
    -days 365 \
    -subj "/CN=localhost"
```

После этого можно запускать HTTPS-сервер:

```bash
./build/async_web_server/async_web_server \
    --protocol https \
    --port 8443 \
    --public async_web_server/public \
    --cert certs/server.crt \
    --key certs/server.key
```

---

## Примеры запуска

### HTTP

```bash
./build/async_web_server/async_web_server --protocol http --port 8080 --public async_web_server/public
```

URL:

```text
http://localhost:8080/
```

### HTTPS

```bash
./build/async_web_server/async_web_server --protocol https --port 8443 --public async_web_server/public --cert certs/server.crt --key certs/server.key
```

URL:

```text
https://localhost:8443/
```

---

## Важное замечание про HTTP и HTTPS

Один запуск сервера работает либо в HTTP-режиме, либо в HTTPS-режиме.

Например:

```text
--protocol http
```

означает, что сервер использует обычный `tcp::socket`.

```text
--protocol https
```

означает, что сервер использует `ssl::stream<tcp::socket>` и выполняет TLS handshake.

Порт сам по себе не определяет протокол. Например, HTTPS можно запустить на порту `8080`, а HTTP — на порту `8443`, но так делать обычно не рекомендуется.

Рекомендуемые порты для локального запуска:

```text
HTTP  -> 8080
HTTPS -> 8443
```

---

## Частые ошибки

### `Not Found`

Если сервер отвечает:

```text
Not Found
```

значит он запустился, но не нашёл запрошенный файл.

Чаще всего проблема в неверном пути к директории `public`.

Например, если запуск выполняется из корня проекта, нужно указывать:

```bash
--public async_web_server/public
```

Проверить наличие файлов можно так:

```bash
ls -la async_web_server/public
```

В директории должен быть файл:

```text
index.html
```

---

### `use_certificate_chain_file: No such file or directory`

Ошибка:

```text
use_certificate_chain_file: No such file or directory
```

означает, что сервер не нашёл файл сертификата.

Проверь пути:

```bash
ls -la certs
```

Должны существовать файлы:

```text
server.crt
server.key
```

Если их нет, создай их:

```bash
mkdir -p certs

openssl req -x509 -newkey rsa:2048 -nodes \
    -keyout certs/server.key \
    -out certs/server.crt \
    -days 365 \
    -subj "/CN=localhost"
```

---

### `--port: command not found`

Если при многострочном запуске появляется ошибка:

```text
--port: command not found
```

значит в предыдущей строке пропущен символ `\`.

Правильно:

```bash
./build/async_web_server/async_web_server \
    --protocol https \
    --port 8443 \
    --public async_web_server/public \
    --cert certs/server.crt \
    --key certs/server.key
```

Неправильно:

```bash
./build/async_web_server/async_web_server \
    --protocol https
    --port 8443
```

---

## Остановка сервера

Сервер можно остановить через `Ctrl+C`.

При этом приложение получает сигнал `SIGINT`, закрывает acceptor и завершает работу worker-потоков.

Пример сообщения:

```text
[server] received signal 2
[server] stopped
```

---

## Краткое описание основных классов

### `WebServer`

Базовый интерфейс сервера.

Содержит общий контракт:

```cpp
virtual boost::asio::awaitable<void> acceptLoop() = 0;
virtual void stop() = 0;
```

---

### `HttpWebServer`

Сервер для обычного HTTP.

Использует:

```cpp
boost::asio::ip::tcp::acceptor
boost::asio::ip::tcp::socket
```

Принимает TCP-подключения и создаёт `HttpSession`.

---

### `HttpsWebServer`

Сервер для HTTPS.

Использует:

```cpp
boost::asio::ip::tcp::acceptor
boost::asio::ssl::context
```

Принимает TCP-подключения и создаёт `HttpsSession`.

---

### `HttpSession`

Клиентская сессия для HTTP.

Работает с обычным TCP-сокетом:

```cpp
tcp::socket
```

---

### `HttpsSession`

Клиентская сессия для HTTPS.

Работает с TLS-потоком:

```cpp
ssl::stream<tcp::socket>
```

Перед чтением HTTP-запросов выполняет TLS handshake.

---

### `HttpRequestHandler`

Связывает сетевой слой и прикладную обработку запроса.

Получает `HttpRequest`, вызывает `Router` и возвращает `HttpResponse`.

---

### `Router`

Маршрутизирует входящий HTTP-запрос.

В текущей реализации передаёт запрос в `StaticFileHandler`.

---

### `StaticFileHandler`

Отвечает за раздачу статических файлов из директории `public`.

Например:

```text
/           -> index.html
/style.css  -> style.css
```

---

### `SslContext`

Настраивает TLS-контекст сервера.

Загружает:

```text
server.crt
server.key
```

---

## Пример полного сценария

Сборка:

```bash
rm -rf build && cmake -S . -B build && cmake --build build --parallel
```

Создание сертификатов:

```bash
mkdir -p certs

openssl req -x509 -newkey rsa:2048 -nodes \
    -keyout certs/server.key \
    -out certs/server.crt \
    -days 365 \
    -subj "/CN=localhost"
```

Запуск HTTP:

```bash
./build/async_web_server/async_web_server \
    --protocol http \
    --port 8080 \
    --public async_web_server/public
```

Запуск HTTPS:

```bash
./build/async_web_server/async_web_server \
    --protocol https \
    --port 8443 \
    --public async_web_server/public \
    --cert certs/server.crt \
    --key certs/server.key
```

Проверка HTTP:

```bash
curl http://localhost:8080/
```

Проверка HTTPS:

```bash
curl -k https://localhost:8443/
```