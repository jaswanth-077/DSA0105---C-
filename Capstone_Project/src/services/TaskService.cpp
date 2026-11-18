#include "TaskService.h"
#include "../database/DatabaseManager.h"
#include <algorithm>

TaskService& TaskService::instance()
{
    static TaskService instance;
    return instance;
}

TaskService::TaskService()
{
}

bool TaskService::validateTask(const Task& task, QString& errorMessage)
{
    if (task.title().trimmed().isEmpty()) {
        errorMessage = "Task title cannot be empty.";
        return false;
    }
    if (task.subject().trimmed().isEmpty()) {
        errorMessage = "Subject cannot be empty.";
        return false;
    }
    if (task.estimatedDuration() <= 0.0) {
        errorMessage = "Estimated duration must be greater than 0 hours.";
        return false;
    }
    if (task.remainingDuration() < 0.0) {
        errorMessage = "Remaining duration cannot be negative.";
        return false;
    }
    if (task.deadline().isNull() || !task.deadline().isValid()) {
        errorMessage = "Please specify a valid deadline.";
        return false;
    }
    return true;
}

int TaskService::createTask(const Task& task, QString& errorMessage)
{
    if (!validateTask(task, errorMessage)) {
        return -1;
    }
    return DatabaseManager::instance().addTask(task);
}

bool TaskService::updateTask(const Task& task, QString& errorMessage)
{
    if (!validateTask(task, errorMessage)) {
        return false;
    }
    return DatabaseManager::instance().updateTask(task);
}

bool TaskService::deleteTask(int taskId)
{
    return DatabaseManager::instance().deleteTask(taskId);
}

std::optional<Task> TaskService::getTask(int taskId)
{
    return DatabaseManager::instance().getTask(taskId);
}

QVector<Task> TaskService::getAllTasks()
{
    return DatabaseManager::instance().getAllTasks();
}

QVector<Task> TaskService::getActiveTasks()
{
    return DatabaseManager::instance().getActiveTasks();
}

QVector<Task> TaskService::getUpcomingTasks(int limit)
{
    QVector<Task> active = getActiveTasks();
    std::sort(active.begin(), active.end(), [](const Task& a, const Task& b) {
        return a.deadline() < b.deadline();
    });
    if (active.size() > limit) {
        active.resize(limit);
    }
    return active;
}

QVector<Task> TaskService::filterTasks(const QString& search,
                                      const QString& subject,
                                      const QString& priority,
                                      const QString& status,
                                      const QString& sortBy)
{
    QVector<Task> tasks = DatabaseManager::instance().getTasksFiltered(search, subject, priority, status);

    if (sortBy == "Deadline (Earliest First)") {
        std::sort(tasks.begin(), tasks.end(), [](const Task& a, const Task& b) {
            return a.deadline() < b.deadline();
        });
    } else if (sortBy == "Deadline (Latest First)") {
        std::sort(tasks.begin(), tasks.end(), [](const Task& a, const Task& b) {
            return a.deadline() > b.deadline();
        });
    } else if (sortBy == "Priority (High to Low)") {
        std::sort(tasks.begin(), tasks.end(), [](const Task& a, const Task& b) {
            return static_cast<int>(a.priority()) > static_cast<int>(b.priority());
        });
    } else if (sortBy == "Priority (Low to High)") {
        std::sort(tasks.begin(), tasks.end(), [](const Task& a, const Task& b) {
            return static_cast<int>(a.priority()) < static_cast<int>(b.priority());
        });
    } else if (sortBy == "Remaining Workload") {
        std::sort(tasks.begin(), tasks.end(), [](const Task& a, const Task& b) {
            return a.remainingDuration() > b.remainingDuration();
        });
    } else if (sortBy == "Completion %") {
        std::sort(tasks.begin(), tasks.end(), [](const Task& a, const Task& b) {
            return a.completionPercentage() > b.completionPercentage();
        });
    }

    return tasks;
}

TaskStatistics TaskService::calculateStatistics()
{
    TaskStatistics stats{};
    QVector<Task> all = getAllTasks();
    stats.totalTasks = all.size();

    double totalWeightedProgress = 0.0;
    QDateTime now = QDateTime::currentDateTime();
    QDateTime oneWeekAgo = now.addDays(-7);
    int tasksDueThisWeek = 0;
    int tasksCompletedThisWeek = 0;

    for (const Task& t : all) {
        stats.totalEstimatedHours += t.estimatedDuration();
        stats.totalRemainingHours += t.remainingDuration();
        totalWeightedProgress += t.completionPercentage();

        if (t.isCompleted()) {
            stats.completedTasks++;
            if (t.deadline() >= oneWeekAgo && t.deadline() <= now.addDays(7)) {
                tasksCompletedThisWeek++;
            }
        } else {
            stats.pendingTasks++;
            if (t.isOverdue()) {
                stats.overdueTasks++;
            } else if (t.status() == TaskStatus::InProgress) {
                stats.inProgressTasks++;
            }
            if (t.priority() == TaskPriority::Critical) {
                stats.criticalTasks++;
            }
        }

        if (t.deadline() >= oneWeekAgo && t.deadline() <= now.addDays(7)) {
            tasksDueThisWeek++;
        }
    }

    if (stats.totalTasks > 0) {
        stats.overallProgressPercent = totalWeightedProgress / static_cast<double>(stats.totalTasks);
    } else {
        stats.overallProgressPercent = 0.0;
    }

    if (tasksDueThisWeek > 0) {
        stats.weeklyCompletionRate = (static_cast<double>(tasksCompletedThisWeek) / static_cast<double>(tasksDueThisWeek)) * 100.0;
    } else if (stats.completedTasks > 0) {
        stats.weeklyCompletionRate = 100.0;
    } else {
        stats.weeklyCompletionRate = 0.0;
    }

    stats.todayPlannedHours = DatabaseManager::instance().getTotalScheduledHoursForDate(QDate::currentDate());

    return stats;
}

QMap<QString, int> TaskService::getPriorityDistribution()
{
    QMap<QString, int> dist;
    dist["Low"] = 0;
    dist["Medium"] = 0;
    dist["High"] = 0;
    dist["Critical"] = 0;

    QVector<Task> active = getActiveTasks();
    for (const Task& t : active) {
        dist[Task::priorityToString(t.priority())]++;
    }
    return dist;
}

QMap<QString, int> TaskService::getStatusDistribution()
{
    QMap<QString, int> dist;
    dist["Not Started"] = 0;
    dist["In Progress"] = 0;
    dist["Completed"] = 0;
    dist["Delayed"] = 0;

    QVector<Task> all = getAllTasks();
    for (const Task& t : all) {
        dist[Task::statusToString(t.status())]++;
    }
    return dist;
}

QMap<QString, double> TaskService::getSubjectWorkloadHours()
{
    QMap<QString, double> hours;
    QVector<Task> active = getActiveTasks();
    for (const Task& t : active) {
        hours[t.subject()] += t.remainingDuration();
    }
    return hours;
}

bool TaskService::updateTaskProgress(int taskId, int newPercentage)
{
    auto opt = getTask(taskId);
    if (!opt) {
        return false;
    }
    Task t = *opt;
    t.setCompletionPercentage(newPercentage);
    if (newPercentage >= 100) {
        t.setStatus(TaskStatus::Completed);
        t.setRemainingDuration(0.0);
    } else {
        double doneRatio = static_cast<double>(newPercentage) / 100.0;
        t.setRemainingDuration(t.estimatedDuration() * (1.0 - doneRatio));
    }
    QString err;
    return updateTask(t, err);
}

bool TaskService::markTaskCompleted(int taskId)
{
    return updateTaskProgress(taskId, 100);
}
