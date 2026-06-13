const elements = {
    form: document.querySelector("#task-form"),
    taskId: document.querySelector("#task-id"),
    title: document.querySelector("#task-title"),
    description: document.querySelector("#task-description"),
    scheduledAt: document.querySelector("#task-scheduled-at"),
    editorTitle: document.querySelector("#editor-title"),
    submitButton: document.querySelector("#submit-button"),
    cancelEditButton: document.querySelector("#cancel-edit-button"),
    refreshButton: document.querySelector("#refresh-button"),
    formMessage: document.querySelector("#form-message"),
    taskCount: document.querySelector("#task-count"),
    loading: document.querySelector("#loading"),
    emptyState: document.querySelector("#empty-state"),
    tasksList: document.querySelector("#tasks-list"),
    taskTemplate: document.querySelector("#task-template")
};

const statusNames = {
    scheduled: "Запланирована",
    running: "Выполняется",
    completed: "Выполнена",
    failed: "Ошибка"
};

let tasks = [];
let refreshTimer = null;

function toDateTimeLocal(timestamp) {
    const date = new Date(timestamp);
    const timezoneOffset = date.getTimezoneOffset() * 60_000;

    return new Date(date.getTime() - timezoneOffset)
        .toISOString()
        .slice(0, 16);
}

function formatDate(timestamp) {
    return new Intl.DateTimeFormat("ru-RU", {
        dateStyle: "medium",
        timeStyle: "short"
    }).format(new Date(timestamp));
}

function setDefaultScheduledAt() {
    elements.scheduledAt.value = toDateTimeLocal(Date.now() + 5 * 60_000);
}

function setFormMessage(message, isError = false) {
    elements.formMessage.textContent = message;
    elements.formMessage.classList.toggle("error", isError);
}

function setFormBusy(isBusy) {
    elements.submitButton.disabled = isBusy;
    elements.cancelEditButton.disabled = isBusy;
}

async function request(url, options = {}) {
    const response = await fetch(url, {
        headers: {
            "Content-Type": "application/json",
            ...(options.headers ?? {})
        },
        ...options
    });

    if (response.status === 204) {
        return null;
    }

    const contentType = response.headers.get("content-type") ?? "";
    const body = contentType.includes("application/json")
        ? await response.json()
        : await response.text();

    if (!response.ok) {
        const message = typeof body === "object" && body?.error
            ? body.error
            : `HTTP ${response.status}`;

        throw new Error(message);
    }

    return body;
}

async function loadTasks({silent = false} = {}) {
    if (!silent) {
        elements.loading.classList.remove("hidden");
        elements.emptyState.classList.add("hidden");
    }

    try {
        tasks = await request("/api/tasks");
        renderTasks();
    } catch (error) {
        elements.tasksList.replaceChildren();
        elements.emptyState.textContent = `Не удалось загрузить задачи: ${error.message}`;
        elements.emptyState.classList.remove("hidden");
    } finally {
        elements.loading.classList.add("hidden");
    }
}

function renderTasks() {
    elements.tasksList.replaceChildren();
    elements.taskCount.textContent = String(tasks.length);
    elements.emptyState.textContent = "Задач пока нет. Создайте первую задачу слева.";
    elements.emptyState.classList.toggle("hidden", tasks.length !== 0);

    for (const task of tasks) {
        const fragment = elements.taskTemplate.content.cloneNode(true);
        const card = fragment.querySelector(".task-card");
        const badge = fragment.querySelector(".status-badge");

        fragment.querySelector(".task-id").textContent = `Задача #${task.id}`;
        fragment.querySelector(".task-title").textContent = task.title;
        fragment.querySelector(".task-description").textContent = task.description;
        fragment.querySelector(".task-scheduled-at").textContent = formatDate(task.scheduledAt);
        fragment.querySelector(".task-updated-at").textContent = formatDate(task.updatedAt);

        badge.textContent = statusNames[task.status] ?? task.status;
        badge.classList.add(`status-${task.status}`);

        const errorElement = fragment.querySelector(".task-error");

        if (task.errorMessage) {
            errorElement.textContent = task.errorMessage;
            errorElement.classList.remove("hidden");
        }

        const runButton = fragment.querySelector(".run-button");
        runButton.disabled = task.status === "running";
        runButton.addEventListener("click", () => runTask(task.id));

        fragment.querySelector(".edit-button")
            .addEventListener("click", () => beginEdit(task));

        fragment.querySelector(".delete-button")
            .addEventListener("click", () => deleteTask(task));

        card.dataset.taskId = String(task.id);
        elements.tasksList.append(fragment);
    }
}

function beginEdit(task) {
    elements.taskId.value = String(task.id);
    elements.title.value = task.title;
    elements.description.value = task.description;
    elements.scheduledAt.value = toDateTimeLocal(task.scheduledAt);
    elements.editorTitle.textContent = `Изменение задачи #${task.id}`;
    elements.submitButton.textContent = "Сохранить изменения";
    elements.cancelEditButton.classList.remove("hidden");
    setFormMessage("");
    elements.title.focus();
}

function resetForm() {
    elements.form.reset();
    elements.taskId.value = "";
    elements.editorTitle.textContent = "Новая задача";
    elements.submitButton.textContent = "Создать задачу";
    elements.cancelEditButton.classList.add("hidden");
    setDefaultScheduledAt();
}

async function saveTask(event) {
    event.preventDefault();
    setFormBusy(true);
    setFormMessage("");

    const id = elements.taskId.value;
    const scheduledAt = new Date(elements.scheduledAt.value).getTime();

    const payload = {
        title: elements.title.value.trim(),
        description: elements.description.value.trim(),
        scheduledAt
    };

    try {
        if (id) {
            await request(`/api/tasks/${id}`, {
                method: "PUT",
                body: JSON.stringify(payload)
            });
            setFormMessage("Задача изменена и перепланирована.");
        } else {
            await request("/api/tasks", {
                method: "POST",
                body: JSON.stringify(payload)
            });
            setFormMessage("Задача создана.");
        }

        resetForm();
        await loadTasks({silent: true});
    } catch (error) {
        setFormMessage(error.message, true);
    } finally {
        setFormBusy(false);
    }
}

async function deleteTask(task) {
    const confirmed = window.confirm(`Удалить задачу «${task.title}»?`);

    if (!confirmed) {
        return;
    }

    try {
        await request(`/api/tasks/${task.id}`, {method: "DELETE"});

        if (elements.taskId.value === String(task.id)) {
            resetForm();
        }

        await loadTasks({silent: true});
    } catch (error) {
        window.alert(`Не удалось удалить задачу: ${error.message}`);
    }
}

async function runTask(id) {
    try {
        await request(`/api/tasks/${id}/run`, {method: "POST"});
        await loadTasks({silent: true});
    } catch (error) {
        window.alert(`Не удалось запустить задачу: ${error.message}`);
    }
}

function startAutoRefresh() {
    window.clearInterval(refreshTimer);
    refreshTimer = window.setInterval(
        () => loadTasks({silent: true}),
        3000);
}

elements.form.addEventListener("submit", saveTask);
elements.cancelEditButton.addEventListener("click", resetForm);
elements.refreshButton.addEventListener("click", () => loadTasks());

setDefaultScheduledAt();
loadTasks();
startAutoRefresh();
