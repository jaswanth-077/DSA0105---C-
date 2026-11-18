#include "DeadlineAnalyzer.h"
#include "../database/DatabaseManager.h"
#include <cmath>
#include <algorithm>

DeadlineAnalyzer& DeadlineAnalyzer::instance()
{
    static DeadlineAnalyzer instance;
    return instance;
}

DeadlineAnalyzer::DeadlineAnalyzer()
{
}

double DeadlineAnalyzer::getDailyCapacity(const QDate& date)
{
    bool isWeekend = (date.dayOfWeek() == Qt::Saturday || date.dayOfWeek() == Qt::Sunday);
    QString key = isWeekend ? "weekend_capacity_hours" : "weekday_capacity_hours";
    double defaultCap = isWeekend ? 6.5 : 5.0;
    QString val = DatabaseManager::instance().getPreference(key, QString::number(defaultCap));
    bool ok = false;
    double res = val.toDouble(&ok);
    return (ok && res > 0.0) ? res : defaultCap;
}

DeadlineRisk DeadlineAnalyzer::analyzeTask(const Task& task)
{
    if (task.isCompleted()) {
        return DeadlineRisk(
            task.id(),
            task.title(),
            task.subject(),
            task.priority(),
            task.deadline(),
            0.0,
            RiskLevel::Safe,
            0.0,
            0.0,
            "Task is completed.",
            "No action needed."
        );
    }

    QDateTime now = QDateTime::currentDateTime();
    double remainingHours = task.remainingDuration();
    if (remainingHours <= 0.0) {
        remainingHours = task.estimatedDuration() * (1.0 - (task.completionPercentage() / 100.0));
    }

    // 1. Check if overdue
    if (now > task.deadline()) {
        qint64 overdueSecs = task.deadline().secsTo(now);
        double overdueHours = static_cast<double>(overdueSecs) / 3600.0;
        QString explanation = QString("Deadline lapsed %1h ago. %2h remaining uncompleted (%3% finished).")
                                  .arg(QString::number(overdueHours, 'f', 1))
                                  .arg(QString::number(remainingHours, 'f', 1))
                                  .arg(task.completionPercentage());
        QString recommendation = "Trigger Schedule Recovery immediately to reserve emergency study blocks.";
        return DeadlineRisk(
            task.id(),
            task.title(),
            task.subject(),
            task.priority(),
            task.deadline(),
            remainingHours,
            RiskLevel::Overdue,
            2.0,
            remainingHours,
            explanation,
            recommendation
        );
    }

    // Calculate days and available hours before deadline
    qint64 totalSecsRemaining = now.secsTo(task.deadline());
    double totalHoursUntilDeadline = static_cast<double>(totalSecsRemaining) / 3600.0;
    double daysUntilDeadline = totalHoursUntilDeadline / 24.0;
    if (daysUntilDeadline < 0.1) {
        daysUntilDeadline = 0.1;
    }

    // Calculate realistic study capacity over the window
    double totalAvailableStudyHours = 0.0;
    QDate currDate = now.date();
    QDate targetDate = task.deadline().date();

    if (currDate == targetDate) {
        // Only today left
        double todayCap = getDailyCapacity(currDate);
        double fractionOfDayLeft = static_cast<double>(now.secsTo(task.deadline())) / 86400.0;
        totalAvailableStudyHours = std::max(0.5, todayCap * std::clamp(fractionOfDayLeft, 0.1, 1.0));
    } else {
        for (QDate d = currDate; d <= targetDate; d = d.addDays(1)) {
            totalAvailableStudyHours += getDailyCapacity(d);
        }
    }

    double urgencyRatio = (totalAvailableStudyHours > 0.0) ? (remainingHours / totalAvailableStudyHours) : 1.5;
    double requiredDailyHours = remainingHours / std::max(0.5, daysUntilDeadline);
    double todayCap = getDailyCapacity(currDate);

    // Classification Logic
    RiskLevel level = RiskLevel::Safe;
    QString explanation;
    QString recommendation;

    // Check Priority Influence
    double criticalThreshold = 0.80;
    double atRiskThreshold = 0.48;

    if (task.priority() == TaskPriority::Critical) {
        criticalThreshold = 0.65;
        atRiskThreshold = 0.38;
    } else if (task.priority() == TaskPriority::High) {
        criticalThreshold = 0.75;
        atRiskThreshold = 0.45;
    }

    if (urgencyRatio >= criticalThreshold || requiredDailyHours >= (todayCap * 0.85) || (daysUntilDeadline <= 1.0 && remainingHours > 3.0)) {
        level = RiskLevel::Critical;
        explanation = QString("High demand: requires %1h in the next %2 days (%3h/day). Available capacity is %4h.")
                          .arg(QString::number(remainingHours, 'f', 1))
                          .arg(QString::number(daysUntilDeadline, 'f', 1))
                          .arg(QString::number(requiredDailyHours, 'f', 1))
                          .arg(QString::number(totalAvailableStudyHours, 'f', 1));
        recommendation = "Severe bottleneck! Prioritize this task now or reschedule lower priority items to avoid missing deadline.";
    } else if (urgencyRatio >= atRiskThreshold || requiredDailyHours >= (todayCap * 0.50) || (daysUntilDeadline <= 2.5 && remainingHours > 4.0)) {
        level = RiskLevel::AtRisk;
        explanation = QString("Requires %1h/day over %2 days to finish on time. Moderate risk if other subjects conflict.")
                          .arg(QString::number(requiredDailyHours, 'f', 1))
                          .arg(QString::number(daysUntilDeadline, 'f', 1));
        recommendation = "Lock in study blocks on your calendar today and tomorrow to maintain safe margin.";
    } else {
        level = RiskLevel::Safe;
        explanation = QString("Well-paced workload (%1h remaining over %2 days, ~%3h/day required).")
                          .arg(QString::number(remainingHours, 'f', 1))
                          .arg(QString::number(daysUntilDeadline, 'f', 1))
                          .arg(QString::number(requiredDailyHours, 'f', 1));
        recommendation = "Current pace is optimal. Keep steady daily increments.";
    }

    return DeadlineRisk(
        task.id(),
        task.title(),
        task.subject(),
        task.priority(),
        task.deadline(),
        remainingHours,
        level,
        urgencyRatio,
        requiredDailyHours,
        explanation,
        recommendation
    );
}

QVector<DeadlineRisk> DeadlineAnalyzer::analyzeAllTasks()
{
    QVector<DeadlineRisk> risks;
    QVector<Task> active = DatabaseManager::instance().getAllTasks();
    for (const Task& t : active) {
        risks.append(analyzeTask(t));
    }

    // Sort by risk severity: Overdue first, then Critical, then AtRisk, then Safe
    std::sort(risks.begin(), risks.end(), [](const DeadlineRisk& a, const DeadlineRisk& b) {
        auto riskScore = [](RiskLevel r) {
            switch (r) {
            case RiskLevel::Overdue:  return 4;
            case RiskLevel::Critical: return 3;
            case RiskLevel::AtRisk:   return 2;
            case RiskLevel::Safe:     return 1;
            }
            return 0;
        };
        int scoreA = riskScore(a.level());
        int scoreB = riskScore(b.level());
        if (scoreA != scoreB) {
            return scoreA > scoreB;
        }
        return a.deadline() < b.deadline();
    });

    return risks;
}

QVector<DeadlineRisk> DeadlineAnalyzer::getCriticalAndAtRiskTasks()
{
    QVector<DeadlineRisk> all = analyzeAllTasks();
    QVector<DeadlineRisk> filtered;
    for (const auto& r : all) {
        if (r.level() == RiskLevel::Critical || r.level() == RiskLevel::Overdue || r.level() == RiskLevel::AtRisk) {
            filtered.append(r);
        }
    }
    return filtered;
}
