#include "DeadlineRisk.h"

DeadlineRisk::DeadlineRisk()
    : m_taskId(-1)
    , m_priority(TaskPriority::Medium)
    , m_remainingDuration(0.0)
    , m_level(RiskLevel::Safe)
    , m_urgencyRatio(0.0)
    , m_requiredDailyHours(0.0)
{
}

DeadlineRisk::DeadlineRisk(int taskId,
                           const QString& taskTitle,
                           const QString& subject,
                           TaskPriority priority,
                           const QDateTime& deadline,
                           double remainingDuration,
                           RiskLevel level,
                           double urgencyRatio,
                           double requiredDailyHours,
                           const QString& explanation,
                           const QString& recommendation)
    : m_taskId(taskId)
    , m_taskTitle(taskTitle)
    , m_subject(subject)
    , m_priority(priority)
    , m_deadline(deadline)
    , m_remainingDuration(remainingDuration)
    , m_level(level)
    , m_urgencyRatio(urgencyRatio)
    , m_requiredDailyHours(requiredDailyHours)
    , m_explanation(explanation)
    , m_recommendation(recommendation)
{
}

QString DeadlineRisk::levelToString(RiskLevel level)
{
    switch (level) {
    case RiskLevel::Safe:     return "Safe";
    case RiskLevel::AtRisk:   return "At Risk";
    case RiskLevel::Critical: return "Critical";
    case RiskLevel::Overdue:  return "Overdue";
    }
    return "Safe";
}

QColor DeadlineRisk::levelColor(RiskLevel level)
{
    switch (level) {
    case RiskLevel::Safe:     return QColor("#10B981"); // Green
    case RiskLevel::AtRisk:   return QColor("#F59E0B"); // Amber
    case RiskLevel::Critical: return QColor("#EF4444"); // Red
    case RiskLevel::Overdue:  return QColor("#DC2626"); // Dark red
    }
    return QColor("#10B981");
}

QColor DeadlineRisk::levelBackgroundColor(RiskLevel level)
{
    switch (level) {
    case RiskLevel::Safe:     return QColor("#ECFDF5"); // Light green
    case RiskLevel::AtRisk:   return QColor("#FFFBEB"); // Light amber
    case RiskLevel::Critical: return QColor("#FEF2F2"); // Light red
    case RiskLevel::Overdue:  return QColor("#FEE2E2"); // Light bright red
    }
    return QColor("#ECFDF5");
}
