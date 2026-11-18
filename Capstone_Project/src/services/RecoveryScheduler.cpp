#include "RecoveryScheduler.h"
#include "DeadlineAnalyzer.h"
#include "../database/DatabaseManager.h"
#include <QMap>
#include <algorithm>
#include <cmath>

RecoveryScheduler& RecoveryScheduler::instance()
{
    static RecoveryScheduler instance;
    return instance;
}

RecoveryScheduler::RecoveryScheduler()
{
}

double RecoveryScheduler::getDayCapacity(const QDate& date)
{
    return DeadlineAnalyzer::instance().getDailyCapacity(date);
}

RecoveryPlan RecoveryScheduler::generateRecoveryPlan(int targetTaskId)
{
    RecoveryPlan plan;
    QDate today = QDate::currentDate();
    int horizonDays = 14;
    QDate endDate = today.addDays(horizonDays);

    // 1. Gather all tasks and existing schedule
    QVector<Task> allTasks = DatabaseManager::instance().getAllTasks();
    QVector<ScheduleEntry> currentEntries = DatabaseManager::instance().getScheduleEntriesBetween(today, endDate);

    // Map daily allocated hours
    QMap<QDate, double> dailyAllocated;
    for (int i = 0; i <= horizonDays; ++i) {
        dailyAllocated[today.addDays(i)] = 0.0;
    }
    for (const auto& entry : currentEntries) {
        dailyAllocated[entry.scheduledDate()] += entry.allocatedHours();
    }

    // 2. Identify tasks needing recovery
    QVector<Task> recoveryCandidates;
    for (const auto& t : allTasks) {
        if (t.isCompleted()) continue;
        if (targetTaskId != -1 && t.id() != targetTaskId) continue;

        DeadlineRisk risk = DeadlineAnalyzer::instance().analyzeTask(t);
        if (risk.level() == RiskLevel::Overdue ||
            risk.level() == RiskLevel::Critical ||
            risk.level() == RiskLevel::AtRisk ||
            t.status() == TaskStatus::Delayed)
        {
            recoveryCandidates.append(t);
        }
    }

    if (recoveryCandidates.isEmpty()) {
        plan.setSummary("All tasks are currently within safe operational margins. No schedule recovery required.");
        plan.setFeasibilityScore(100.0);
        return plan;
    }

    // Sort candidates by priority & deadline urgency
    std::sort(recoveryCandidates.begin(), recoveryCandidates.end(), [](const Task& a, const Task& b) {
        if (a.isOverdue() != b.isOverdue()) {
            return a.isOverdue() > b.isOverdue();
        }
        if (a.priority() != b.priority()) {
            return static_cast<int>(a.priority()) > static_cast<int>(b.priority());
        }
        return a.deadline() < b.deadline();
    });

    int resolvedCount = 0;
    double totalNeededHours = 0.0;
    double totalAccommodatedHours = 0.0;

    // 3. Process each candidate
    for (const auto& task : recoveryCandidates) {
        // Calculate already scheduled hours for this task
        double alreadyScheduled = 0.0;
        for (const auto& entry : currentEntries) {
            if (entry.taskId() == task.id()) {
                alreadyScheduled += entry.allocatedHours();
            }
        }

        double needed = task.remainingDuration() - alreadyScheduled;
        if (needed <= 0.25) {
            continue; // Already adequately scheduled
        }

        totalNeededHours += needed;
        QDate taskDeadlineDate = task.deadline().date();
        if (taskDeadlineDate < today) {
            // Overdue - distribute across next 3 days
            taskDeadlineDate = today.addDays(2);
        }

        double remainingToPlace = needed;

        // Try to allocate in blocks of 1.0 - 2.0 hours
        for (QDate d = today; d <= taskDeadlineDate && remainingToPlace > 0.1; d = d.addDays(1)) {
            double cap = getDayCapacity(d);
            double currentLoad = dailyAllocated[d];
            double freeSpace = cap - currentLoad;

            if (freeSpace >= 0.75) {
                double block = std::min(remainingToPlace, std::min(2.0, freeSpace));
                if (block > 0.25) {
                    RescheduleAction action;
                    action.taskId = task.id();
                    action.taskTitle = task.title();
                    action.subject = task.subject();
                    action.priority = task.priority();
                    action.originalDate = QDate(); // New slot
                    action.proposedDate = d;
                    action.allocatedHours = block;
                    action.isNewAllocation = true;
                    action.reason = QString("Recovery allocation: prioritize %1 before %2 deadline")
                                        .arg(Task::priorityToString(task.priority()))
                                        .arg(task.isOverdue() ? "lapsed" : "upcoming");

                    plan.addAction(action);
                    dailyAllocated[d] += block;
                    remainingToPlace -= block;
                    totalAccommodatedHours += block;
                    resolvedCount++;
                }
            } else if (task.priority() >= TaskPriority::High) {
                // Look for lower-priority entries on day 'd' to reschedule later
                for (auto& entry : currentEntries) {
                    if (entry.scheduledDate() == d &&
                        static_cast<int>(entry.priority()) < static_cast<int>(task.priority()) &&
                        entry.taskId() != task.id())
                    {
                        // Reschedule entry to a future day after d
                        QDate newDay = d.addDays(1);
                        while (newDay <= endDate && (dailyAllocated[newDay] + entry.allocatedHours() > getDayCapacity(newDay))) {
                            newDay = newDay.addDays(1);
                        }
                        if (newDay <= endDate) {
                            // Record shift of the lower priority task
                            RescheduleAction shiftAction;
                            shiftAction.taskId = entry.taskId();
                            shiftAction.taskTitle = entry.taskTitle();
                            shiftAction.subject = entry.subject();
                            shiftAction.priority = entry.priority();
                            shiftAction.originalDate = d;
                            shiftAction.proposedDate = newDay;
                            shiftAction.allocatedHours = entry.allocatedHours();
                            shiftAction.isNewAllocation = false;
                            shiftAction.reason = QString("Deferred to accommodate urgent recovery of \"%1\"").arg(task.title());

                            plan.addAction(shiftAction);
                            dailyAllocated[d] -= entry.allocatedHours();
                            dailyAllocated[newDay] += entry.allocatedHours();
                            entry.setScheduledDate(newDay);

                            // Now allocate space to the urgent task
                            double freed = entry.allocatedHours();
                            double block = std::min(remainingToPlace, std::min(2.0, freed));
                            RescheduleAction urgentAction;
                            urgentAction.taskId = task.id();
                            urgentAction.taskTitle = task.title();
                            urgentAction.subject = task.subject();
                            urgentAction.priority = task.priority();
                            urgentAction.originalDate = QDate();
                            urgentAction.proposedDate = d;
                            urgentAction.allocatedHours = block;
                            urgentAction.isNewAllocation = true;
                            urgentAction.reason = QString("Emergency slot reclaimed from lower-priority task");

                            plan.addAction(urgentAction);
                            dailyAllocated[d] += block;
                            remainingToPlace -= block;
                            totalAccommodatedHours += block;
                            resolvedCount++;
                            break;
                        }
                    }
                }
            }
        }
    }

    plan.setConflictsResolved(resolvedCount);
    double score = 100.0;
    if (totalNeededHours > 0.0) {
        score = std::min(100.0, (totalAccommodatedHours / totalNeededHours) * 100.0);
    }
    plan.setFeasibilityScore(score);

    QString summaryText = QString("Recovery algorithm generated %1 schedule adjustment(s). "
                                  "Accommodated %2h of %3h required recovery workload (Feasibility: %4%). "
                                  "Daily limits preserved without silently changing original deadlines.")
                              .arg(plan.totalAdjustments())
                              .arg(QString::number(totalAccommodatedHours, 'f', 1))
                              .arg(QString::number(totalNeededHours, 'f', 1))
                              .arg(QString::number(score, 'f', 0));
    plan.setSummary(summaryText);

    return plan;
}

bool RecoveryScheduler::applyRecoveryPlan(const RecoveryPlan& plan)
{
    if (plan.isEmpty()) {
        return true;
    }

    for (const auto& action : plan.actions()) {
        if (action.isNewAllocation) {
            ScheduleEntry entry(
                -1,
                action.taskId,
                action.taskTitle,
                action.subject,
                action.priority,
                action.proposedDate,
                action.allocatedHours,
                false,
                action.reason
            );
            DatabaseManager::instance().addScheduleEntry(entry);
        } else {
            // Find and update the existing entry
            QVector<ScheduleEntry> entries = DatabaseManager::instance().getScheduleEntriesForDate(action.originalDate);
            for (auto& e : entries) {
                if (e.taskId() == action.taskId) {
                    e.setScheduledDate(action.proposedDate);
                    e.setNotes(action.reason);
                    DatabaseManager::instance().updateScheduleEntry(e);
                    break;
                }
            }
        }

        // If the task was marked Delayed, switch to In Progress once recovery blocks are added
        auto opt = DatabaseManager::instance().getTask(action.taskId);
        if (opt && opt->status() == TaskStatus::Delayed) {
            Task t = *opt;
            t.setStatus(TaskStatus::InProgress);
            DatabaseManager::instance().updateTask(t);
        }
    }

    return true;
}
