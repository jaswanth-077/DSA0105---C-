#pragma once

#include <QVector>
#include <QString>
#include <QMap>
#include <optional>
#include "../models/Task.h"

struct TaskStatistics {
    int totalTasks;
    int completedTasks;
    int pendingTasks;
    int overdueTasks;
    int inProgressTasks;
    int criticalTasks;
    double overallProgressPercent;
    double weeklyCompletionRate;
    double totalEstimatedHours;
    double totalRemainingHours;
    double todayPlannedHours;
};

class TaskService {
public:
    static TaskService& instance();

    int createTask(const Task& task, QString& errorMessage);
    bool updateTask(const Task& task, QString& errorMessage);
    bool deleteTask(int taskId);
    std::optional<Task> getTask(int taskId);
    QVector<Task> getAllTasks();
    QVector<Task> getActiveTasks();
    QVector<Task> getUpcomingTasks(int limit = 5);

    QVector<Task> filterTasks(const QString& search,
                              const QString& subject,
                              const QString& priority,
                              const QString& status,
                              const QString& sortBy);

    TaskStatistics calculateStatistics();
    QMap<QString, int> getPriorityDistribution();
    QMap<QString, int> getStatusDistribution();
    QMap<QString, double> getSubjectWorkloadHours();

    bool updateTaskProgress(int taskId, int newPercentage);
    bool markTaskCompleted(int taskId);

private:
    TaskService();
    ~TaskService() = default;
    TaskService(const TaskService&) = delete;
    TaskService& operator=(const TaskService&) = delete;

    bool validateTask(const Task& task, QString& errorMessage);
};
