const apiUrl = "/api/tasks";
const $ = (id) => document.getElementById(id);

const form = $("task-form");
const taskId = $("task-id");
const title = $("task-title");
const description = $("task-description");
const scheduledAt = $("task-scheduled-at");
const scheduledSecond = $("task-scheduled-second");
const formTitle = $("form-title");
const submitButton = $("submit-button");
const cancelEditButton = $("cancel-edit-button");
const refreshButton = $("refresh-button");
const formMessage = $("form-message");
const loading = $("loading");
const emptyState = $("empty-state");
const tasksList = $("tasks-list");
const taskCount = $("task-count");
const taskTemplate = $("task-template");

let tasks = [];

//------------------------------------------------------------------------------

function showMessage(text, isError = false) {
    formMessage.textContent = text;
    formMessage.classList.toggle("error", isError);
}

//------------------------------------------------------------------------------

function setLoading(value) {
    loading.classList.toggle("hidden", !value);
    refreshButton.disabled = value;
    submitButton.disabled = value;
}

//------------------------------------------------------------------------------

async function apiRequest(url, options = {}) {
    const response = await fetch(url, {
        headers: {
            "Content-Type": "application/json"
        },
        ...options
    });

    const text = await response.text();

    if (!response.ok) {
        throw new Error(text || `HTTP error ${response.status}`);
    }

    return text ? JSON.parse(text) : null;
}

//------------------------------------------------------------------------------

function toTimestampMs(value, secondValue) {
    const date = new Date(value);

    if (Number.isNaN(date.getTime())) {
        return null;
    }

    const seconds = Number(secondValue);

    if (!Number.isInteger(seconds) || seconds < 0 || seconds > 59) {
        return null;
    }

    date.setSeconds(seconds, 0);

    return date.getTime();
}

//------------------------------------------------------------------------------

function toDateTimeLocalValue(value) {
    if (!value) {
        return "";
    }

    const date = new Date(Number(value));

    if (Number.isNaN(date.getTime())) {
        return "";
    }

    const offsetMs = date.getTimezoneOffset() * 60 * 1000;
    const localDate = new Date(date.getTime() - offsetMs);

    return localDate.toISOString().slice(0, 16);
}

//------------------------------------------------------------------------------

function toSecondValue(value) {
    if (!value) {
        return "0";
    }

    const date = new Date(Number(value));

    if (Number.isNaN(date.getTime())) {
        return "0";
    }

    return String(date.getSeconds());
}

//------------------------------------------------------------------------------

function formatDateTime(value) {
    if (!value) {
        return "Не указано";
    }

    const date = new Date(Number(value));

    if (Number.isNaN(date.getTime())) {
        return String(value);
    }

    return date.toLocaleString("ru-RU", {
        year: "numeric",
        month: "2-digit",
        day: "2-digit",
        hour: "2-digit",
        minute: "2-digit",
        second: "2-digit"
    });
}

//------------------------------------------------------------------------------

function formatStatus(status) {
    if (status === "scheduled") {
        return "ЗАПЛАНИРОВАНО";
    }

    if (status === "running") {
        return "ВЫПОЛНЯЕТСЯ";
    }

    if (status === "completed") {
        return "ИСПОЛНЕНО";
    }

    if (status === "failed") {
        return "ОШИБКА";
    }

    return status || "";
}

//------------------------------------------------------------------------------

function resetForm() {
    form.reset();
    taskId.value = "";
    scheduledSecond.value = "0";

    formTitle.textContent = "Создать задачу";
    submitButton.textContent = "Создать задачу";
    cancelEditButton.classList.add("hidden");

    showMessage("");
}

//------------------------------------------------------------------------------

function startEdit(task) {
    taskId.value = task.id;
    title.value = task.title || "";
    description.value = task.description || "";
    scheduledAt.value = toDateTimeLocalValue(task.scheduledAt);
    scheduledSecond.value = toSecondValue(task.scheduledAt);

    formTitle.textContent = "Изменить задачу";
    submitButton.textContent = "Сохранить изменения";
    cancelEditButton.classList.remove("hidden");

    showMessage("Измените параметры задачи и сохраните изменения.");
}

//------------------------------------------------------------------------------

function renderTasks() {
    tasksList.innerHTML = "";
    taskCount.textContent = String(tasks.length);
    emptyState.classList.toggle("hidden", tasks.length > 0);

    for (const task of tasks) {
        const node = taskTemplate.content.cloneNode(true);

        node.querySelector(".task-id").textContent = `ID: ${task.id}`;
        node.querySelector(".task-title").textContent =
            task.title || "Без названия";
        node.querySelector(".task-description").textContent =
            task.description || "";
        node.querySelector(".task-scheduled-at").textContent =
            formatDateTime(task.scheduledAt);

        const statusElement = node.querySelector(".task-status, .status-badge");

        if (statusElement) {
            statusElement.textContent = formatStatus(task.status);
        }

        const errorElement = node.querySelector(".task-error");

        if (errorElement) {
            errorElement.textContent = task.errorMessage || "";
            errorElement.classList.toggle("hidden", !task.errorMessage);
        }

        node.querySelector(".edit-button").addEventListener("click", () => {
            startEdit(task);
        });

        node.querySelector(".delete-button").addEventListener(
            "click",
            async () => {
                await deleteTask(task.id);
            });

        tasksList.appendChild(node);
    }
}

//------------------------------------------------------------------------------

async function loadTasks() {
    setLoading(true);

    try {
        const response = await apiRequest(apiUrl);

        tasks = Array.isArray(response) ? response : response.tasks || [];

        renderTasks();
        showMessage("Список задач обновлён.");
    } catch (error) {
        showMessage(`Ошибка получения списка задач: ${error.message}`, true);
    } finally {
        setLoading(false);
    }
}

//------------------------------------------------------------------------------

async function deleteTask(id) {
    if (!window.confirm("Удалить задачу?")) {
        return;
    }

    try {
        await apiRequest(`${apiUrl}/${id}`, {
            method: "DELETE"
        });

        showMessage("Задача удалена.");
        await loadTasks();
    } catch (error) {
        showMessage(`Ошибка удаления задачи: ${error.message}`, true);
    }
}

//------------------------------------------------------------------------------

form.addEventListener("submit", async (event) => {
    event.preventDefault();

    const payload = {
        title: title.value.trim(),
        description: description.value.trim(),
        scheduledAt: toTimestampMs(
            scheduledAt.value,
            scheduledSecond.value)
    };

    if (!payload.title) {
        showMessage("Введите название задачи.", true);
        return;
    }

    if (payload.scheduledAt === null) {
        showMessage("Укажите дату, время и секунды выполнения.", true);
        return;
    }

    const id = taskId.value;
    const url = id ? `${apiUrl}/${id}` : apiUrl;
    const method = id ? "PUT" : "POST";

    setLoading(true);

    try {
        await apiRequest(url, {
            method,
            body: JSON.stringify(payload)
        });

        showMessage(id ? "Задача изменена." : "Задача создана.");
        resetForm();
        await loadTasks();
    } catch (error) {
        showMessage(`Ошибка сохранения задачи: ${error.message}`, true);
    } finally {
        setLoading(false);
    }
});

//------------------------------------------------------------------------------

cancelEditButton.addEventListener("click", resetForm);

//------------------------------------------------------------------------------

refreshButton.addEventListener("click", loadTasks);

//------------------------------------------------------------------------------

document.addEventListener("DOMContentLoaded", loadTasks);

//------------------------------------------------------------------------------