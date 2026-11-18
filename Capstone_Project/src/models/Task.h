#pragma once

#include <QString>
#include <QDateTime>
#include <QColor>

enum class TaskType {
    Assignment,
    ExamPrep,
    Project,
    Revision,
    PersonalStudy
};

enum class TaskPriority {
    Low,
    Medium,
    High,
    Critical
};

enum class TaskStatus {
    NotStarted,
    InProgress,
    Completed,
    Delayed
};

class Task {
public:
    Task();
    Task(int id,
         const QString& title,
         const QString& description,
         const QString& subject,
         TaskType type,
         TaskPriority priority,
         const QDateTime& createdAt,
         const QDateTime& deadline,
         double estimatedDuration,
         double remainingDuration,
         int completionPercentage,
         TaskStatus status);

    // Getters
    int id() const { return m_id; }
    QString title() const { return m_title; }
    QString description() const { return m_description; }
    QString subject() const { return m_subject; }
    TaskType type() const { return m_type; }
    TaskPriority priority() const { return m_priority; }
    QDateTime createdAt() const { return m_createdAt; }
    QDateTime deadline() const { return m_deadline; }
    double estimatedDuration() const { return m_estimatedDuration; }
    double remainingDuration() const { return m_remainingDuration; }
    int completionPercentage() const { return m_completionPercentage; }
    TaskStatus status() const { return m_status; }

    // Setters
    void setId(int id) { m_id = id; }
    void setTitle(const QString& title) { m_title = title; }
    void setDescription(const QString& description) { m_description = description; }
    void setSubject(const QString& subject) { m_subject = subject; }
    void setType(TaskType type) { m_type = type; }
    void setPriority(TaskPriority priority) { m_priority = priority; }
    void setCreatedAt(const QDateTime& dt) { m_createdAt = dt; }
    void setDeadline(const QDateTime& dt) { m_deadline = dt; }
    void setEstimatedDuration(double hours) { m_estimatedDuration = hours; }
    void setRemainingDuration(double hours) { m_remainingDuration = hours; }
    void setCompletionPercentage(int percent);
    void setStatus(TaskStatus status) { m_status = status; }

    // Helpers
    bool isCompleted() const;
    bool isOverdue() const;
    double hoursRemainingBeforeDeadline() const;

    // Type String utilities
    static QString typeToString(TaskType type);
    static TaskType stringToType(const QString& str);

    // Priority String & Color utilities
    static QString priorityToString(TaskPriority priority);
    static TaskPriority stringToPriority(const QString& str);
    static QColor priorityColor(TaskPriority priority);

    // Status String & Color utilities
    static QString statusToString(TaskStatus status);
    static TaskStatus stringToStatus(const QString& str);
    static QColor statusColor(TaskStatus status);

private:
    int m_id;
    QString m_title;
    QString m_description;
    QString m_subject;
    TaskType m_type;
    TaskPriority m_priority;
    QDateTime m_createdAt;
    QDateTime m_deadline;
    double m_estimatedDuration;
    double m_remainingDuration;
    int m_completionPercentage;
    TaskStatus m_status;
};
