#pragma once

#include <QString>
#include <QColor>
#include "Task.h"

enum class RiskLevel {
    Safe,
    AtRisk,
    Critical,
    Overdue
};

class DeadlineRisk {
public:
    DeadlineRisk();
    DeadlineRisk(int taskId,
                 const QString& taskTitle,
                 const QString& subject,
                 TaskPriority priority,
                 const QDateTime& deadline,
                 double remainingDuration,
                 RiskLevel level,
                 double urgencyRatio,
                 double requiredDailyHours,
                 const QString& explanation,
                 const QString& recommendation);

    int taskId() const { return m_taskId; }
    QString taskTitle() const { return m_taskTitle; }
    QString subject() const { return m_subject; }
    TaskPriority priority() const { return m_priority; }
    QDateTime deadline() const { return m_deadline; }
    double remainingDuration() const { return m_remainingDuration; }
    RiskLevel level() const { return m_level; }
    double urgencyRatio() const { return m_urgencyRatio; }
    double requiredDailyHours() const { return m_requiredDailyHours; }
    QString explanation() const { return m_explanation; }
    QString recommendation() const { return m_recommendation; }

    static QString levelToString(RiskLevel level);
    static QColor levelColor(RiskLevel level);
    static QColor levelBackgroundColor(RiskLevel level);

private:
    int m_taskId;
    QString m_taskTitle;
    QString m_subject;
    TaskPriority m_priority;
    QDateTime m_deadline;
    double m_remainingDuration;
    RiskLevel m_level;
    double m_urgencyRatio;
    double m_requiredDailyHours;
    QString m_explanation;
    QString m_recommendation;
};
