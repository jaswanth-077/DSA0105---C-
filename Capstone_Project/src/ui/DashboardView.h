#pragma once

#include <QWidget>
#include <QLabel>
#include <QTableWidget>
#include "widgets/StatCard.h"
#include "widgets/WorkloadChartWidget.h"
#include "widgets/StatusPieChartWidget.h"

class DashboardView : public QWidget {
    Q_OBJECT

public:
    explicit DashboardView(QWidget* parent = nullptr);

    void refreshData();

signals:
    void newTaskRequested();
    void openRecoveryRequested();
    void openTaskManagerRequested();

private slots:
    void onUpcomingTaskDoubleClicked(int row, int col);
    void onQuickActionClicked();

private:
    void setupUi();
    void populateUpcomingDeadlines();
    void populateDeadlineAlerts();

    // Stat Cards
    StatCard* m_activeTasksCard;
    StatCard* m_completedTasksCard;
    StatCard* m_pendingTasksCard;
    StatCard* m_overdueTasksCard;
    StatCard* m_todayWorkloadCard;
    StatCard* m_weeklyRateCard;

    // Charts
    WorkloadChartWidget* m_workloadChart;
    StatusPieChartWidget* m_priorityChart;

    // Upcoming Deadlines Table
    QTableWidget* m_deadlinesTable;

    // Deadline Risk Banner/List
    QWidget* m_riskBannerContainer;
    QVBoxLayout* m_riskListLayout;
};
