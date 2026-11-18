#include "DashboardView.h"
#include "../../services/TaskService.h"
#include "../../services/DeadlineAnalyzer.h"
#include "../../services/WorkloadBalancer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QPushButton>
#include <QHeaderView>
#include <QFrame>

DashboardView::DashboardView(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
    refreshData();
}

void DashboardView::setupUi()
{
    auto* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("background-color: transparent;");

    auto* contentWidget = new QWidget(scrollArea);
    auto* mainLayout = new QVBoxLayout(contentWidget);
    mainLayout->setContentsMargins(30, 25, 30, 30);
    mainLayout->setSpacing(24);

    // Header with greetings and action buttons
    auto* headerLayout = new QHBoxLayout();
    auto* titleContainer = new QWidget(contentWidget);
    auto* titleLayout = new QVBoxLayout(titleContainer);
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(4);

    auto* title = new QLabel("Academic Dashboard", titleContainer);
    title->setProperty("class", "pageTitle");
    auto* subtitle = new QLabel("Real-time workload overview, deadline tracking, and performance analytics.", titleContainer);
    subtitle->setProperty("class", "pageSubtitle");

    titleLayout->addWidget(title);
    titleLayout->addWidget(subtitle);
    headerLayout->addWidget(titleContainer);
    headerLayout->addStretch();

    auto* recoverBtn = new QPushButton("⚡ Run Recovery Assistant", contentWidget);
    recoverBtn->setProperty("class", "secondaryButton");
    connect(recoverBtn, &QPushButton::clicked, this, &DashboardView::openRecoveryRequested);

    auto* newTaskBtn = new QPushButton("+ New Task", contentWidget);
    newTaskBtn->setProperty("class", "primaryButton");
    connect(newTaskBtn, &QPushButton::clicked, this, &DashboardView::newTaskRequested);

    headerLayout->addWidget(recoverBtn);
    headerLayout->addWidget(newTaskBtn);
    mainLayout->addLayout(headerLayout);

    // Stat Cards Grid (6 cards)
    auto* statsGrid = new QGridLayout();
    statsGrid->setSpacing(14);

    m_activeTasksCard = new StatCard("🎯", "Active Tasks", "0", "In-progress & scheduled", "#4F46E5", contentWidget);
    m_completedTasksCard = new StatCard("✅", "Completed", "0", "0% overall completion", "#10B981", contentWidget);
    m_pendingTasksCard = new StatCard("⏳", "Pending", "0", "Unfinished assignments", "#6366F1", contentWidget);
    m_overdueTasksCard = new StatCard("⚠️", "Overdue", "0", "Requires recovery", "#EF4444", contentWidget);
    m_todayWorkloadCard = new StatCard("⏱️", "Today's Study", "0.0h", "Against daily limit", "#F59E0B", contentWidget);
    m_weeklyRateCard = new StatCard("📈", "Weekly Rate", "0%", "Tasks completed on time", "#06B6D4", contentWidget);

    statsGrid->addWidget(m_activeTasksCard, 0, 0);
    statsGrid->addWidget(m_completedTasksCard, 0, 1);
    statsGrid->addWidget(m_pendingTasksCard, 0, 2);
    statsGrid->addWidget(m_overdueTasksCard, 1, 0);
    statsGrid->addWidget(m_todayWorkloadCard, 1, 1);
    statsGrid->addWidget(m_weeklyRateCard, 1, 2);
    mainLayout->addLayout(statsGrid);

    // Charts Row (Workload Bar Chart + Status/Priority Donut)
    auto* chartsRow = new QHBoxLayout();
    chartsRow->setSpacing(20);

    // Left Chart Card: 7-Day Workload Bar Chart
    auto* workloadCard = new QFrame(contentWidget);
    workloadCard->setProperty("class", "cardFrame");
    auto* wlLayout = new QVBoxLayout(workloadCard);
    wlLayout->setContentsMargins(20, 18, 20, 18);
    wlLayout->setSpacing(12);

    auto* wlHeader = new QLabel("7-Day Workload Distribution & Daily Capacity", workloadCard);
    wlHeader->setProperty("class", "sectionTitle");
    m_workloadChart = new WorkloadChartWidget(workloadCard);
    wlLayout->addWidget(wlHeader);
    wlLayout->addWidget(m_workloadChart);

    // Right Chart Card: Task Distribution Donut
    auto* pieCard = new QFrame(contentWidget);
    pieCard->setProperty("class", "cardFrame");
    auto* pieLayout = new QVBoxLayout(pieCard);
    pieLayout->setContentsMargins(20, 18, 20, 18);
    pieLayout->setSpacing(12);

    auto* pieHeader = new QLabel("Priority Distribution", pieCard);
    pieHeader->setProperty("class", "sectionTitle");
    m_priorityChart = new StatusPieChartWidget(pieCard);
    pieLayout->addWidget(pieHeader);
    pieLayout->addWidget(m_priorityChart);

    chartsRow->addWidget(workloadCard, 3);
    chartsRow->addWidget(pieCard, 2);
    mainLayout->addLayout(chartsRow);

    // Bottom Row: Upcoming Deadlines + Deadline Risk Alerts
    auto* bottomRow = new QHBoxLayout();
    bottomRow->setSpacing(20);

    // Left Bottom: Upcoming Deadlines Table
    auto* deadlinesCard = new QFrame(contentWidget);
    deadlinesCard->setProperty("class", "cardFrame");
    auto* dlLayout = new QVBoxLayout(deadlinesCard);
    dlLayout->setContentsMargins(20, 18, 20, 18);
    dlLayout->setSpacing(12);

    auto* dlHeaderLayout = new QHBoxLayout();
    auto* dlHeader = new QLabel("Upcoming Deadlines", deadlinesCard);
    dlHeader->setProperty("class", "sectionTitle");
    auto* viewAllBtn = new QPushButton("View All Tasks →", deadlinesCard);
    viewAllBtn->setProperty("class", "secondaryButton");
    connect(viewAllBtn, &QPushButton::clicked, this, &DashboardView::openTaskManagerRequested);

    dlHeaderLayout->addWidget(dlHeader);
    dlHeaderLayout->addStretch();
    dlHeaderLayout->addWidget(viewAllBtn);
    dlLayout->addLayout(dlHeaderLayout);

    m_deadlinesTable = new QTableWidget(deadlinesCard);
    m_deadlinesTable->setColumnCount(5);
    m_deadlinesTable->setHorizontalHeaderLabels({"Task Title", "Subject", "Priority", "Deadline", "Remaining"});
    m_deadlinesTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_deadlinesTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_deadlinesTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_deadlinesTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_deadlinesTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_deadlinesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_deadlinesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_deadlinesTable->verticalHeader()->setVisible(false);
    m_deadlinesTable->setMinimumHeight(180);
    connect(m_deadlinesTable, &QTableWidget::cellDoubleClicked, this, &DashboardView::onUpcomingTaskDoubleClicked);
    dlLayout->addWidget(m_deadlinesTable);

    // Right Bottom: Deadline Risk Alerts
    auto* riskCard = new QFrame(contentWidget);
    riskCard->setProperty("class", "cardFrame");
    auto* rkLayout = new QVBoxLayout(riskCard);
    rkLayout->setContentsMargins(20, 18, 20, 18);
    rkLayout->setSpacing(12);

    auto* rkHeader = new QLabel("Deadline Risk Assessment", riskCard);
    rkHeader->setProperty("class", "sectionTitle");
    rkLayout->addWidget(rkHeader);

    m_riskBannerContainer = new QWidget(riskCard);
    m_riskListLayout = new QVBoxLayout(m_riskBannerContainer);
    m_riskListLayout->setContentsMargins(0, 0, 0, 0);
    m_riskListLayout->setSpacing(8);
    rkLayout->addWidget(m_riskBannerContainer);
    rkLayout->addStretch();

    bottomRow->addWidget(deadlinesCard, 3);
    bottomRow->addWidget(riskCard, 2);
    mainLayout->addLayout(bottomRow);

    scrollArea->setWidget(contentWidget);
    outerLayout->addWidget(scrollArea);
}

void DashboardView::refreshData()
{
    // Update Stat Cards
    TaskStatistics stats = TaskService::instance().calculateStatistics();
    m_activeTasksCard->setValue(QString::number(stats.totalTasks - stats.completedTasks));
    m_completedTasksCard->setValue(QString::number(stats.completedTasks));
    m_completedTasksCard->setSubtext(QString("%1% overall completion").arg(stats.overallProgressPercent, 0, 'f', 0));
    m_pendingTasksCard->setValue(QString::number(stats.pendingTasks));
    m_overdueTasksCard->setValue(QString::number(stats.overdueTasks));
    if (stats.overdueTasks > 0) {
        m_overdueTasksCard->setSubtext("Critical action required!");
    } else {
        m_overdueTasksCard->setSubtext("All deadlines healthy");
    }
    m_todayWorkloadCard->setValue(QString("%1h").arg(stats.todayPlannedHours, 0, 'f', 1));
    m_weeklyRateCard->setValue(QString("%1%").arg(stats.weeklyCompletionRate, 0, 'f', 0));

    // Update Workload Chart
    WorkloadAnalysis wa = WorkloadBalancer::instance().analyzeWorkload(QDate::currentDate(), 7);
    QVector<ChartDayData> chartData;
    for (const auto& dw : wa.dailyWorkloads) {
        ChartDayData cdd;
        cdd.date = dw.date;
        cdd.dayName = dw.date.toString("ddd");
        cdd.scheduledHours = dw.scheduledHours;
        cdd.capacityHours = dw.capacityHours;
        cdd.isOverloaded = dw.isOverloaded;
        chartData.append(cdd);
    }
    m_workloadChart->setData(chartData);

    // Update Priority Donut Chart
    QMap<QString, int> prioDist = TaskService::instance().getPriorityDistribution();
    QVector<PieSlice> slices = {
        {"Critical", prioDist["Critical"], QColor("#EF4444")},
        {"High", prioDist["High"], QColor("#F59E0B")},
        {"Medium", prioDist["Medium"], QColor("#3B82F6")},
        {"Low", prioDist["Low"], QColor("#64748B")}
    };
    m_priorityChart->setSlices(slices, "Active");

    populateUpcomingDeadlines();
    populateDeadlineAlerts();
}

void DashboardView::populateUpcomingDeadlines()
{
    QVector<Task> upcoming = TaskService::instance().getUpcomingTasks(5);
    m_deadlinesTable->setRowCount(upcoming.size());

    for (int r = 0; r < upcoming.size(); ++r) {
        const auto& t = upcoming[r];

        auto* titleItem = new QTableWidgetItem(t.title());
        titleItem->setData(Qt::UserRole, t.id());
        m_deadlinesTable->setItem(r, 0, titleItem);

        m_deadlinesTable->setItem(r, 1, new QTableWidgetItem(t.subject()));

        auto* prioItem = new QTableWidgetItem(Task::priorityToString(t.priority()));
        prioItem->setForeground(Task::priorityColor(t.priority()));
        m_deadlinesTable->setItem(r, 2, prioItem);

        QString dlStr = t.deadline().toString("MMM d, HH:mm");
        if (t.isOverdue()) {
            dlStr += " (Overdue)";
        }
        auto* dlItem = new QTableWidgetItem(dlStr);
        if (t.isOverdue()) {
            dlItem->setForeground(QColor("#DC2626"));
            dlItem->setFont(QFont("Segoe UI", 9, QFont::Bold));
        }
        m_deadlinesTable->setItem(r, 3, dlItem);

        m_deadlinesTable->setItem(r, 4, new QTableWidgetItem(QString("%1h").arg(t.remainingDuration(), 0, 'f', 1)));
    }
}

void DashboardView::populateDeadlineAlerts()
{
    // Clear previous items
    QLayoutItem* item;
    while ((item = m_riskListLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }

    QVector<DeadlineRisk> risks = DeadlineAnalyzer::instance().getCriticalAndAtRiskTasks();
    if (risks.isEmpty()) {
        auto* goodBanner = new QLabel("✨ All deadlines are currently safe and well-paced.", m_riskBannerContainer);
        goodBanner->setStyleSheet("background-color: #ECFDF5; color: #065F46; border: 1px solid #A7F3D0; border-radius: 8px; padding: 12px; font-weight: 500;");
        m_riskListLayout->addWidget(goodBanner);
        return;
    }

    for (const auto& r : risks) {
        auto* box = new QFrame(m_riskBannerContainer);
        QString bgColor = (r.level() == RiskLevel::Overdue || r.level() == RiskLevel::Critical) ? "#FEF2F2" : "#FFFBEB";
        QString borderColor = (r.level() == RiskLevel::Overdue || r.level() == RiskLevel::Critical) ? "#FECACA" : "#FDE68A";
        box->setStyleSheet(QString("background-color: %1; border: 1px solid %2; border-radius: 8px; padding: 10px;").arg(bgColor, borderColor));

        auto* bLayout = new QVBoxLayout(box);
        bLayout->setContentsMargins(8, 8, 8, 8);
        bLayout->setSpacing(4);

        auto* rowTop = new QHBoxLayout();
        auto* title = new QLabel(r.taskTitle(), box);
        title->setStyleSheet("font-weight: 700; color: #1E293B; font-size: 13px;");

        auto* badge = new QLabel(DeadlineRisk::levelToString(r.level()), box);
        badge->setStyleSheet(QString("background-color: %1; color: #FFFFFF; font-weight: bold; border-radius: 4px; padding: 2px 6px; font-size: 10px;").arg(DeadlineRisk::levelColor(r.level()).name()));

        rowTop->addWidget(title);
        rowTop->addStretch();
        rowTop->addWidget(badge);
        bLayout->addLayout(rowTop);

        auto* exp = new QLabel(r.explanation(), box);
        exp->setWordWrap(true);
        exp->setStyleSheet("color: #475569; font-size: 12px;");
        bLayout->addWidget(exp);

        m_riskListLayout->addWidget(box);
    }
}

void DashboardView::onUpcomingTaskDoubleClicked(int row, int /*col*/)
{
    emit openTaskManagerRequested();
}

void DashboardView::onQuickActionClicked()
{
}
