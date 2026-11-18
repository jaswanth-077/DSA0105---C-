#include "WorkloadBalancer.h"
#include "DeadlineAnalyzer.h"
#include "../database/DatabaseManager.h"
#include <algorithm>

WorkloadBalancer& WorkloadBalancer::instance()
{
    static WorkloadBalancer instance;
    return instance;
}

WorkloadBalancer::WorkloadBalancer()
{
}

double WorkloadBalancer::getCapacityForDate(const QDate& date)
{
    return DeadlineAnalyzer::instance().getDailyCapacity(date);
}

WorkloadAnalysis WorkloadBalancer::analyzeWorkload(const QDate& startDate, int dayCount)
{
    WorkloadAnalysis analysis;
    analysis.totalScheduledHours = 0.0;
    analysis.totalCapacityHours = 0.0;
    analysis.overloadedDaysCount = 0;

    QDate endDate = startDate.addDays(dayCount - 1);
    QVector<ScheduleEntry> entries = DatabaseManager::instance().getScheduleEntriesBetween(startDate, endDate);

    for (int i = 0; i < dayCount; ++i) {
        QDate date = startDate.addDays(i);
        DailyWorkload dw;
        dw.date = date;
        dw.capacityHours = getCapacityForDate(date);
        dw.scheduledHours = 0.0;

        for (const auto& e : entries) {
            if (e.scheduledDate() == date) {
                dw.entries.append(e);
                dw.scheduledHours += e.allocatedHours();
            }
        }

        dw.isOverloaded = (dw.scheduledHours > dw.capacityHours + 0.1);
        if (dw.isOverloaded) {
            analysis.overloadedDaysCount++;
        }

        analysis.totalScheduledHours += dw.scheduledHours;
        analysis.totalCapacityHours += dw.capacityHours;
        analysis.dailyWorkloads.append(dw);
    }

    if (dayCount > 0) {
        analysis.averageDailyLoad = analysis.totalScheduledHours / static_cast<double>(dayCount);
    } else {
        analysis.averageDailyLoad = 0.0;
    }

    // Generate smart recommendations
    for (const auto& dw : analysis.dailyWorkloads) {
        if (dw.isOverloaded) {
            double excess = dw.scheduledHours - dw.capacityHours;
            // Look for adjacent day with slack
            for (const auto& other : analysis.dailyWorkloads) {
                if (!other.isOverloaded && (other.capacityHours - other.scheduledHours) >= excess) {
                    analysis.recommendations.append(
                        QString("Overload on %1 (+%2h over capacity). Recommend shifting flexible tasks to %3.")
                            .arg(dw.date.toString("ddd, MMM d"))
                            .arg(QString::number(excess, 'f', 1))
                            .arg(other.date.toString("ddd, MMM d"))
                    );
                    break;
                }
            }
        }
    }

    if (analysis.recommendations.isEmpty()) {
        if (analysis.totalScheduledHours == 0.0) {
            analysis.recommendations.append("No tasks scheduled for this period. Add tasks to maintain regular study momentum.");
        } else {
            analysis.recommendations.append("Workload is evenly distributed within daily thresholds. Great pacing!");
        }
    }

    return analysis;
}

QVector<SubjectProgress> WorkloadBalancer::getSubjectProgressSummary()
{
    QVector<SubjectProgress> summary;
    QVector<QString> subjects = DatabaseManager::instance().getDistinctSubjects();
    QVector<Task> allTasks = DatabaseManager::instance().getAllTasks();

    for (const QString& sub : subjects) {
        SubjectProgress sp;
        sp.subject = sub;
        sp.totalTasks = 0;
        sp.completedTasks = 0;
        sp.estimatedHours = 0.0;
        sp.remainingHours = 0.0;
        sp.actualLoggedHours = 0.0;

        for (const auto& t : allTasks) {
            if (t.subject() == sub) {
                sp.totalTasks++;
                if (t.isCompleted()) {
                    sp.completedTasks++;
                }
                sp.estimatedHours += t.estimatedDuration();
                sp.remainingHours += t.remainingDuration();
                sp.actualLoggedHours += DatabaseManager::instance().getTotalActualHoursForTask(t.id());
            }
        }

        if (sp.totalTasks > 0) {
            sp.completionRatePercent = (static_cast<double>(sp.completedTasks) / static_cast<double>(sp.totalTasks)) * 100.0;
        } else {
            sp.completionRatePercent = 0.0;
        }

        summary.append(sp);
    }

    return summary;
}

QMap<QString, double> WorkloadBalancer::getEstimatedVsActualHours()
{
    QMap<QString, double> comp;
    double totalEst = 0.0;
    double totalAct = 0.0;

    QVector<Task> allTasks = DatabaseManager::instance().getAllTasks();
    for (const auto& t : allTasks) {
        totalEst += t.estimatedDuration();
        totalAct += DatabaseManager::instance().getTotalActualHoursForTask(t.id());
    }

    comp["Estimated"] = totalEst;
    comp["Actual"] = totalAct;
    return comp;
}

bool WorkloadBalancer::balanceOverloadedDays(QString& summaryReport)
{
    WorkloadAnalysis analysis = analyzeWorkload(QDate::currentDate(), 14);
    if (analysis.overloadedDaysCount == 0) {
        summaryReport = "All days are currently within healthy workload thresholds. No redistribution needed.";
        return true;
    }

    int shiftedCount = 0;
    double totalHoursShifted = 0.0;

    for (const auto& dw : analysis.dailyWorkloads) {
        if (!dw.isOverloaded) continue;

        double excess = dw.scheduledHours - dw.capacityHours;
        QVector<ScheduleEntry> entries = dw.entries;

        // Sort entries by priority ascending so lowest priority moves first
        std::sort(entries.begin(), entries.end(), [](const ScheduleEntry& a, const ScheduleEntry& b) {
            return static_cast<int>(a.priority()) < static_cast<int>(b.priority());
        });

        for (auto& entry : entries) {
            if (excess <= 0.1) break;

            // Find subsequent day with available capacity
            for (const auto& targetDay : analysis.dailyWorkloads) {
                if (targetDay.date > dw.date && !targetDay.isOverloaded) {
                    double targetFree = targetDay.capacityHours - targetDay.scheduledHours;
                    if (targetFree >= entry.allocatedHours()) {
                        entry.setScheduledDate(targetDay.date);
                        entry.setNotes(QString("Auto-balanced: shifted from overloaded %1").arg(dw.date.toString("MMM d")));
                        DatabaseManager::instance().updateScheduleEntry(entry);

                        shiftedCount++;
                        totalHoursShifted += entry.allocatedHours();
                        excess -= entry.allocatedHours();
                        break;
                    }
                }
            }
        }
    }

    summaryReport = QString("Successfully balanced schedule: moved %1 study session(s) (%2h total) from peak overloaded days to open slots.")
                        .arg(shiftedCount)
                        .arg(QString::number(totalHoursShifted, 'f', 1));
    return true;
}
