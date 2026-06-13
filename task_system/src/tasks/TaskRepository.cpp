#include "tasks/TaskRepository.hpp"

#include <sqlite3.h>

#include <cstdint>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace tasks
{

namespace
{

class Statement
{
public:
    Statement(
        sqlite3          *database,
        const std::string &sql)
    {
        const int result = sqlite3_prepare_v2(
            database,
            sql.c_str(),
            -1,
            &statement_,
            nullptr);

        if (result != SQLITE_OK)
        {
            throw std::runtime_error(
                "sqlite3_prepare_v2 failed: " +
                std::string(sqlite3_errmsg(database)));
        }
    }

    ~Statement()
    {
        sqlite3_finalize(statement_);
    }

    Statement(const Statement &) = delete;
    Statement &operator=(const Statement &) = delete;

    sqlite3_stmt *get() const
    {
        return statement_;
    }

private:
    sqlite3_stmt *statement_ = nullptr;
};

//------------------------------------------------------------------------------

void checkResult(
    sqlite3    *database,
    int         result,
    const char *operation)
{
    if (result == SQLITE_OK ||
        result == SQLITE_ROW ||
        result == SQLITE_DONE)
    {
        return;
    }

    throw std::runtime_error(
        std::string(operation) + " failed: " +
        sqlite3_errmsg(database));
}

//------------------------------------------------------------------------------

void bindInt64(
    sqlite3         *database,
    sqlite3_stmt    *statement,
    int              index,
    std::int64_t     value)
{
    checkResult(
        database,
        sqlite3_bind_int64(statement, index, value),
        "sqlite3_bind_int64");
}

//------------------------------------------------------------------------------

void bindText(
    sqlite3          *database,
    sqlite3_stmt     *statement,
    int               index,
    const std::string &value)
{
    checkResult(
        database,
        sqlite3_bind_text(
            statement,
            index,
            value.c_str(),
            static_cast<int>(value.size()),
            SQLITE_TRANSIENT),
        "sqlite3_bind_text");
}

//------------------------------------------------------------------------------

std::string columnText(
    sqlite3_stmt *statement,
    int           index)
{
    const auto *text = sqlite3_column_text(statement, index);

    if (text == nullptr)
    {
        return {};
    }

    return reinterpret_cast<const char *>(text);
}

//------------------------------------------------------------------------------

Task readTask(sqlite3_stmt *statement)
{
    Task task;

    task.id              = sqlite3_column_int64(statement, 0);
    task.title           = columnText(statement, 1);
    task.description     = columnText(statement, 2);
    task.scheduled_at_ms = sqlite3_column_int64(statement, 3);
    task.status          = taskStatusFromString(columnText(statement, 4));

    if (sqlite3_column_type(statement, 5) != SQLITE_NULL)
    {
        task.error_message = columnText(statement, 5);
    }

    task.created_at_ms = sqlite3_column_int64(statement, 6);
    task.updated_at_ms = sqlite3_column_int64(statement, 7);

    return task;
}

//------------------------------------------------------------------------------

std::optional<Task> findByIdUnlocked(
    sqlite3 *database,
    TaskId   id)
{
    Statement statement(
        database,
        R"(
            SELECT
                id,
                title,
                description,
                scheduled_at_ms,
                status,
                error_message,
                created_at_ms,
                updated_at_ms
            FROM tasks
            WHERE id = ?;
        )");

    bindInt64(database, statement.get(), 1, id);

    const int result = sqlite3_step(statement.get());

    if (result == SQLITE_DONE)
    {
        return std::nullopt;
    }

    checkResult(database, result, "sqlite3_step");

    return readTask(statement.get());
}

} // namespace

//------------------------------------------------------------------------------

TaskRepository::TaskRepository(
    const std::filesystem::path &database_path)
{
    if (database_path.has_parent_path())
    {
        std::filesystem::create_directories(
            database_path.parent_path());
    }

    const int result = sqlite3_open_v2(
        database_path.string().c_str(),
        &database_,
        SQLITE_OPEN_READWRITE |
            SQLITE_OPEN_CREATE |
            SQLITE_OPEN_FULLMUTEX,
        nullptr);

    if (result != SQLITE_OK)
    {
        const std::string error = database_ != nullptr
            ? sqlite3_errmsg(database_)
            : "unknown SQLite error";

        if (database_ != nullptr)
        {
            sqlite3_close(database_);
            database_ = nullptr;
        }

        throw std::runtime_error(
            "Unable to open SQLite database: " + error);
    }

    sqlite3_busy_timeout(database_, 5000);

    createSchema();
}

//------------------------------------------------------------------------------

TaskRepository::~TaskRepository()
{
    if (database_ != nullptr)
    {
        sqlite3_close(database_);
    }
}

//------------------------------------------------------------------------------

Task TaskRepository::create(const TaskInput &input)
{
    std::lock_guard<std::mutex> lock(mutex_);

    const std::int64_t now = currentUnixTimeMs();

    Statement statement(
        database_,
        R"(
            INSERT INTO tasks
            (
                title,
                description,
                scheduled_at_ms,
                status,
                error_message,
                created_at_ms,
                updated_at_ms
            )
            VALUES (?, ?, ?, 'scheduled', NULL, ?, ?);
        )");

    bindText(database_, statement.get(), 1, input.title);
    bindText(database_, statement.get(), 2, input.description);
    bindInt64(database_, statement.get(), 3, input.scheduled_at_ms);
    bindInt64(database_, statement.get(), 4, now);
    bindInt64(database_, statement.get(), 5, now);

    checkResult(
        database_,
        sqlite3_step(statement.get()),
        "sqlite3_step");

    const TaskId id = sqlite3_last_insert_rowid(database_);

    const std::optional<Task> task = findByIdUnlocked(database_, id);

    if (!task.has_value())
    {
        throw std::runtime_error("Created task was not found");
    }

    return *task;
}

//------------------------------------------------------------------------------

std::optional<Task> TaskRepository::findById(TaskId id) const
{
    std::lock_guard<std::mutex> lock(mutex_);

    return findByIdUnlocked(database_, id);
}

//------------------------------------------------------------------------------

std::vector<Task> TaskRepository::findAll() const
{
    std::lock_guard<std::mutex> lock(mutex_);

    Statement statement(
        database_,
        R"(
            SELECT
                id,
                title,
                description,
                scheduled_at_ms,
                status,
                error_message,
                created_at_ms,
                updated_at_ms
            FROM tasks
            ORDER BY scheduled_at_ms ASC, id ASC;
        )");

    std::vector<Task> tasks;

    for (;;)
    {
        const int result = sqlite3_step(statement.get());

        if (result == SQLITE_DONE)
        {
            break;
        }

        checkResult(database_, result, "sqlite3_step");

        tasks.push_back(readTask(statement.get()));
    }

    return tasks;
}

//------------------------------------------------------------------------------

std::vector<Task> TaskRepository::findScheduled() const
{
    std::lock_guard<std::mutex> lock(mutex_);

    Statement statement(
        database_,
        R"(
            SELECT
                id,
                title,
                description,
                scheduled_at_ms,
                status,
                error_message,
                created_at_ms,
                updated_at_ms
            FROM tasks
            WHERE status = 'scheduled'
            ORDER BY scheduled_at_ms ASC, id ASC;
        )");

    std::vector<Task> tasks;

    for (;;)
    {
        const int result = sqlite3_step(statement.get());

        if (result == SQLITE_DONE)
        {
            break;
        }

        checkResult(database_, result, "sqlite3_step");

        tasks.push_back(readTask(statement.get()));
    }

    return tasks;
}

//------------------------------------------------------------------------------

std::optional<Task> TaskRepository::update(
    TaskId          id,
    const TaskInput &input)
{
    std::lock_guard<std::mutex> lock(mutex_);

    Statement statement(
        database_,
        R"(
            UPDATE tasks
            SET
                title = ?,
                description = ?,
                scheduled_at_ms = ?,
                status = 'scheduled',
                error_message = NULL,
                updated_at_ms = ?
            WHERE id = ?;
        )");

    bindText(database_, statement.get(), 1, input.title);
    bindText(database_, statement.get(), 2, input.description);
    bindInt64(database_, statement.get(), 3, input.scheduled_at_ms);
    bindInt64(database_, statement.get(), 4, currentUnixTimeMs());
    bindInt64(database_, statement.get(), 5, id);

    checkResult(
        database_,
        sqlite3_step(statement.get()),
        "sqlite3_step");

    if (sqlite3_changes(database_) == 0)
    {
        return std::nullopt;
    }

    return findByIdUnlocked(database_, id);
}

//------------------------------------------------------------------------------

bool TaskRepository::remove(TaskId id)
{
    std::lock_guard<std::mutex> lock(mutex_);

    Statement statement(
        database_,
        "DELETE FROM tasks WHERE id = ?;");

    bindInt64(database_, statement.get(), 1, id);

    checkResult(
        database_,
        sqlite3_step(statement.get()),
        "sqlite3_step");

    return sqlite3_changes(database_) != 0;
}

//------------------------------------------------------------------------------

bool TaskRepository::claimForExecution(TaskId id)
{
    std::lock_guard<std::mutex> lock(mutex_);

    Statement statement(
        database_,
        R"(
            UPDATE tasks
            SET
                status = 'running',
                error_message = NULL,
                updated_at_ms = ?
            WHERE id = ?
              AND status = 'scheduled';
        )");

    bindInt64(database_, statement.get(), 1, currentUnixTimeMs());
    bindInt64(database_, statement.get(), 2, id);

    checkResult(
        database_,
        sqlite3_step(statement.get()),
        "sqlite3_step");

    return sqlite3_changes(database_) == 1;
}

//------------------------------------------------------------------------------

void TaskRepository::setStatus(
    TaskId                     id,
    TaskStatus                 status,
    std::optional<std::string> error_message)
{
    std::lock_guard<std::mutex> lock(mutex_);

    Statement statement(
        database_,
        R"(
            UPDATE tasks
            SET
                status = ?,
                error_message = ?,
                updated_at_ms = ?
            WHERE id = ?;
        )");

    bindText(database_, statement.get(), 1, toString(status));

    if (error_message.has_value())
    {
        bindText(database_, statement.get(), 2, *error_message);
    }
    else
    {
        checkResult(
            database_,
            sqlite3_bind_null(statement.get(), 2),
            "sqlite3_bind_null");
    }

    bindInt64(database_, statement.get(), 3, currentUnixTimeMs());
    bindInt64(database_, statement.get(), 4, id);

    checkResult(
        database_,
        sqlite3_step(statement.get()),
        "sqlite3_step");
}

//------------------------------------------------------------------------------

void TaskRepository::createSchema()
{
    execute("PRAGMA journal_mode = WAL;");
    execute("PRAGMA foreign_keys = ON;");

    execute(
        R"(
            CREATE TABLE IF NOT EXISTS tasks
            (
                id              INTEGER PRIMARY KEY AUTOINCREMENT,
                title           TEXT    NOT NULL,
                description     TEXT    NOT NULL DEFAULT '',
                scheduled_at_ms INTEGER NOT NULL,
                status          TEXT    NOT NULL
                    CHECK (status IN (
                        'scheduled',
                        'running',
                        'completed',
                        'failed'
                    )),
                error_message   TEXT,
                created_at_ms   INTEGER NOT NULL,
                updated_at_ms   INTEGER NOT NULL
            );
        )");

    execute(
        R"(
            CREATE INDEX IF NOT EXISTS idx_tasks_status_scheduled_at
            ON tasks(status, scheduled_at_ms);
        )");
}

//------------------------------------------------------------------------------

void TaskRepository::execute(const std::string &sql) const
{
    std::lock_guard<std::mutex> lock(mutex_);

    char *error_message = nullptr;

    const int result = sqlite3_exec(
        database_,
        sql.c_str(),
        nullptr,
        nullptr,
        &error_message);

    if (result != SQLITE_OK)
    {
        const std::string message = error_message != nullptr
            ? error_message
            : sqlite3_errmsg(database_);

        sqlite3_free(error_message);

        throw std::runtime_error(
            "sqlite3_exec failed: " + message);
    }
}

} // namespace tasks
