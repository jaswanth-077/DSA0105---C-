#pragma once

#include <QVector>
#include <QDate>
#include "../models/RecoveryPlan.h"
#include "../models/Task.h"
#include "../models/ScheduleEntry.h"

class RecoveryScheduler {
public:
    static RecoveryScheduler& instance();

    RecoveryPlan generateRecoveryPlan(int targetTaskId = -1);
    bool applyRecoveryPlan(const RecoveryPlan& plan);

private:
    RecoveryScheduler();
    ~RecoveryScheduler() = default;
    RecoveryScheduler(const RecoveryScheduler&) = delete;
    RecoveryScheduler& operator=(const RecoveryScheduler&) = delete;

    double getDayCapacity(const QDate& date);
};
