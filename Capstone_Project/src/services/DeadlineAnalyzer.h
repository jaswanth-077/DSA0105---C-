#pragma once

#include <QVector>
#include <QDateTime>
#include "../models/DeadlineRisk.h"
#include "../models/Task.h"

class DeadlineAnalyzer {
public:
    static DeadlineAnalyzer& instance();

    DeadlineRisk analyzeTask(const Task& task);
    QVector<DeadlineRisk> analyzeAllTasks();
    QVector<DeadlineRisk> getCriticalAndAtRiskTasks();

    double getDailyCapacity(const QDate& date);

private:
    DeadlineAnalyzer();
    ~DeadlineAnalyzer() = default;
    DeadlineAnalyzer(const DeadlineAnalyzer&) = delete;
    DeadlineAnalyzer& operator=(const DeadlineAnalyzer&) = delete;
};
