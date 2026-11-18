#pragma once

#include <QString>
#include <QDate>
#include <QVector>
#include "Task.h"

struct RescheduleAction {
    int taskId;
    QString taskTitle;
    QString subject;
    TaskPriority priority;
    QDate originalDate;
    QDate proposedDate;
    double allocatedHours;
    QString reason;
    bool isNewAllocation; // true if newly assigned slot, false if moved
};

class RecoveryPlan {
public:
    RecoveryPlan();

    void addAction(const RescheduleAction& action);
    const QVector<RescheduleAction>& actions() const { return m_actions; }

    int totalAdjustments() const { return m_actions.size(); }
    int conflictsResolved() const { return m_conflictsResolved; }
    void setConflictsResolved(int count) { m_conflictsResolved = count; }

    double feasibilityScore() const { return m_feasibilityScore; }
    void setFeasibilityScore(double score) { m_feasibilityScore = score; }

    QString summary() const { return m_summary; }
    void setSummary(const QString& summary) { m_summary = summary; }

    bool isEmpty() const { return m_actions.isEmpty(); }

private:
    QVector<RescheduleAction> m_actions;
    int m_conflictsResolved;
    double m_feasibilityScore;
    QString m_summary;
};
