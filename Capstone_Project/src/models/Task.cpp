#include "Task.h"
#include <algorithm>

Task::Task()
    : m_id(-1)
    , m_type(TaskType::Assignment)
    , m_priority(TaskPriority::Medium)
    , m_createdAt(QDateTime::currentDateTime())
    , m_deadline(QDateTime::currentDateTime().addDays(3))
    , m_estimatedDuration(2.0)
    , m_remainingDuration(2.0)
    , m_completionPercentage(0)
    , m_status(TaskStatus::NotStarted)
{
}

Task::Task(int id,
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
           TaskStatus status)
    : m_id(id)
    , m_title(title)
    , m_description(description)
    , m_subject(subject)
    , m_type(type)
    , m_priority(priority)
    , m_createdAt(createdAt)
    , m_deadline(deadline)
    , m_estimatedDuration(estimatedDuration)
    , m_remainingDuration(remainingDuration)
    , m_completionPercentage(std::clamp(completionPercentage, 0, 100))
    , m_status(status)
{
}

void Task::setCompletionPercentage(int percent)
{
    m_completionPercentage = std::clamp(percent, 0, 100);
    if (m_completionPercentage == 100) {
        m_status = TaskStatus::Completed;
        m_remainingDuration = 0.0;
    } else if (m_completionPercentage > 0 && m_status == TaskStatus::NotStarted) {
        m_status = TaskStatus::InProgress;
    }
}

bool Task::isCompleted() const
{
    return m_completionPercentage >= 100 || m_status == TaskStatus::Completed;
}

bool Task::isOverdue() const
{
    if (isCompleted()) {
        return false;
    }
    return QDateTime::currentDateTime() > m_deadline;
}

double Task::hoursRemainingBeforeDeadline() const
{
    qint64 secs = QDateTime::currentDateTime().secsTo(m_deadline);
    if (secs < 0) {
        return 0.0;
    }
    return static_cast<double>(secs) / 3600.0;
}

QString Task::typeToString(TaskType type)
{
    switch (type) {
    case TaskType::Assignment:    return "Assignment";
    case TaskType::ExamPrep:      return "Exam Preparation";
    case TaskType::Project:       return "Project";
    case TaskType::Revision:      return "Revision";
    case TaskType::PersonalStudy: return "Personal Study";
    }
    return "Assignment";
}

TaskType Task::stringToType(const QString& str)
{
    if (str == "Exam Preparation") return TaskType::ExamPrep;
    if (str == "Project") return TaskType::Project;
    if (str == "Revision") return TaskType::Revision;
    if (str == "Personal Study") return TaskType::PersonalStudy;
    return TaskType::Assignment;
}

QString Task::priorityToString(TaskPriority priority)
{
    switch (priority) {
    case TaskPriority::Low:      return "Low";
    case TaskPriority::Medium:   return "Medium";
    case TaskPriority::High:     return "High";
    case TaskPriority::Critical: return "Critical";
    }
    return "Medium";
}

TaskPriority Task::stringToPriority(const QString& str)
{
    if (str == "Low") return TaskPriority::Low;
    if (str == "High") return TaskPriority::High;
    if (str == "Critical") return TaskPriority::Critical;
    return TaskPriority::Medium;
}

QColor Task::priorityColor(TaskPriority priority)
{
    switch (priority) {
    case TaskPriority::Low:      return QColor("#64748B"); // Slate gray
    case TaskPriority::Medium:   return QColor("#3B82F6"); // Blue
    case TaskPriority::High:     return QColor("#F59E0B"); // Amber
    case TaskPriority::Critical: return QColor("#EF4444"); // Red
    }
    return QColor("#64748B");
}

QString Task::statusToString(TaskStatus status)
{
    switch (status) {
    case TaskStatus::NotStarted: return "Not Started";
    case TaskStatus::InProgress: return "In Progress";
    case TaskStatus::Completed:  return "Completed";
    case TaskStatus::Delayed:    return "Delayed";
    }
    return "Not Started";
}

TaskStatus Task::stringToStatus(const QString& str)
{
    if (str == "In Progress") return TaskStatus::InProgress;
    if (str == "Completed") return TaskStatus::Completed;
    if (str == "Delayed") return TaskStatus::Delayed;
    return TaskStatus::NotStarted;
}

QColor Task::statusColor(TaskStatus status)
{
    switch (status) {
    case TaskStatus::NotStarted: return QColor("#94A3B8"); // Cool gray
    case TaskStatus::InProgress: return QColor("#6366F1"); // Indigo
    case TaskStatus::Completed:  return QColor("#10B981"); // Emerald green
    case TaskStatus::Delayed:    return QColor("#F43F5E"); // Rose red
    }
    return QColor("#94A3B8");
}
