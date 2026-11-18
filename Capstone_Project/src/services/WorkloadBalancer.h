#pragma once

#include <QDate>
#include <QVector>
#include <QString>
#include <QMap>
#include "../models/ScheduleEntry.h"

struct DailyWorkload {
    QDate date;
    double scheduledHours;
    double capacityHours;
    bool isOverloaded;
    QVector<ScheduleEntry> entries;
};

struct WorkloadAnalysis {
    QVector<DailyWorkload> dailyWorkloads;
    double totalScheduledHours;
    double totalCapacityHours;
    double averageDailyLoad;
    int overloadedDaysCount;
    QVector<QString> recommendations;
};

struct SubjectProgress {
    QString subject;
    int totalTasks;
    int completedTasks;
    double estimatedHours;
    double remainingHours;
    double actualLoggedHours;
    double completionRatePercent;
};

class WorkloadBalancer {
public:
    static WorkloadBalancer& instance();

    WorkloadAnalysis analyzeWorkload(const QDate& startDate = QDate::currentDate(), int dayCount = 7);
    QVector<SubjectProgress> getSubjectProgressSummary();
    QMap<QString, double> getEstimatedVsActualHours();

    bool balanceOverloadedDays(QString& summaryReport);

private:
    WorkloadBalancer();
    ~WorkloadBalancer() = default;
    WorkloadBalancer(const WorkloadBalancer&) = delete;
    WorkloadBalancer& operator=(const WorkloadBalancer&) = delete;

    double getCapacityForDate(const QDate& date);
};
