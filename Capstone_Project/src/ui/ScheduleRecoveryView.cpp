#include "ScheduleRecoveryView.h"
#include "../../services/DeadlineAnalyzer.h"
#include "../../services/RecoveryScheduler.h"
#include "../../services/WorkloadBalancer.h"
#include "../../database/DatabaseManager.h"
#include "dialogs/RecoveryPlanDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QFrame>

ScheduleRecoveryView::ScheduleRecoveryView(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
    refreshData();
}

void ScheduleRecoveryView::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 25, 30, 30);
    mainLayout->setSpacing(24);

    // Header Row
    auto* headerLayout = new QHBoxLayout();
    auto* titleContainer = new QWidget(this);
    auto* titleLayout = new QVBoxLayout(titleContainer);
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(4);

    auto* title = new QLabel("Deadline Risk Analysis & Schedule Recovery", titleContainer);
    title->setProperty("class", "pageTitle");
    auto* subtitle = new QLabel("Identify high-risk assignments, detect delayed progress, and automatically generate optimal recovery schedules.", titleContainer);
    subtitle->setProperty("class", "pageSubtitle");

    titleLayout->addWidget(title);
    titleLayout->addWidget(subtitle);
    headerLayout->addWidget(titleContainer);
    headerLayout->addStretch();

    m_balanceBtn = new QPushButton("⚖️ Balance Daily Overload", this);
    m_balanceBtn->setProperty("class", "secondaryButton");
    connect(m_balanceBtn, &QPushButton::clicked, this, &ScheduleRecoveryView::onAutoBalanceClicked);

    m_recoveryBtn = new QPushButton("⚡ Run Schedule Recovery", this);
    m_recoveryBtn->setProperty("class", "primaryButton");
    connect(m_recoveryBtn, &QPushButton::clicked, this, &ScheduleRecoveryView::onRunRecoveryAssistantClicked);

    headerLayout->addWidget(m_balanceBtn);
    headerLayout->addWidget(m_recoveryBtn);
    mainLayout->addLayout(headerLayout);

    // Section 1: Deadline Risk Assessment Table Card
    auto* riskCard = new QFrame(this);
    riskCard->setProperty("class", "cardFrame");
    auto* rkLayout = new QVBoxLayout(riskCard);
    rkLayout->setContentsMargins(20, 18, 20, 18);
    rkLayout->setSpacing(12);

    auto* rkHeader = new QLabel("Active Task Risk Evaluation", riskCard);
    rkHeader->setProperty("class", "sectionTitle");
    rkLayout->addWidget(rkHeader);

    m_riskTable = new QTableWidget(riskCard);
    m_riskTable->setColumnCount(6);
    m_riskTable->setHorizontalHeaderLabels({"Task Title", "Subject", "Priority", "Deadline", "Risk Level", "Algorithmic Analysis & Recommendation"});
    m_riskTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_riskTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_riskTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_riskTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_riskTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_riskTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
    m_riskTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_riskTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_riskTable->verticalHeader()->setVisible(false);
    m_riskTable->setMinimumHeight(200);
    rkLayout->addWidget(m_riskTable);
    mainLayout->addWidget(riskCard);

    // Section 2: Daily Scheduled Study Blocks Card
    auto* schedCard = new QFrame(this);
    schedCard->setProperty("class", "cardFrame");
    auto* scLayout = new QVBoxLayout(schedCard);
    scLayout->setContentsMargins(20, 18, 20, 18);
    scLayout->setSpacing(12);

    auto* scHeaderLayout = new QHBoxLayout();
    auto* scHeader = new QLabel("Daily Study Calendar & Scheduled Slots", schedCard);
    scHeader->setProperty("class", "sectionTitle");

    m_datePicker = new QDateEdit(QDate::currentDate(), schedCard);
    m_datePicker->setCalendarPopup(true);
    m_datePicker->setDisplayFormat("yyyy-MM-dd (ddd)");
    connect(m_datePicker, &QDateEdit::dateChanged, this, &ScheduleRecoveryView::onScheduleDateChanged);

    m_dayWorkloadLabel = new QLabel("Total Scheduled: 0.0h", schedCard);
    m_dayWorkloadLabel->setStyleSheet("font-weight: 600; color: #4F46E5;");

    scHeaderLayout->addWidget(scHeader);
    scHeaderLayout->addStretch();
    scHeaderLayout->addWidget(new QLabel("Selected Date:", schedCard));
    scHeaderLayout->addWidget(m_datePicker);
    scHeaderLayout->addSpacing(12);
    scHeaderLayout->addWidget(m_dayWorkloadLabel);
    scLayout->addLayout(scHeaderLayout);

    m_scheduleTable = new QTableWidget(schedCard);
    m_scheduleTable->setColumnCount(6);
    m_scheduleTable->setHorizontalHeaderLabels({"Task Title", "Subject", "Priority", "Allocated Workload", "Status", "Notes / Purpose"});
    m_scheduleTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_scheduleTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_scheduleTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_scheduleTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_scheduleTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_scheduleTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
    m_scheduleTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_scheduleTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_scheduleTable->verticalHeader()->setVisible(false);
    m_scheduleTable->setMinimumHeight(180);
    scLayout->addWidget(m_scheduleTable);

    // Action buttons for scheduled entries
    auto* entryActionLayout = new QHBoxLayout();
    entryActionLayout->addStretch();

    auto* completeEntryBtn = new QPushButton("Mark Slot Completed", schedCard);
    completeEntryBtn->setProperty("class", "secondaryButton");
    connect(completeEntryBtn, &QPushButton::clicked, this, &ScheduleRecoveryView::onMarkEntryCompletedClicked);

    auto* deleteEntryBtn = new QPushButton("Remove Slot", schedCard);
    deleteEntryBtn->setProperty("class", "secondaryButton");
    connect(deleteEntryBtn, &QPushButton::clicked, this, &ScheduleRecoveryView::onDeleteScheduleEntryClicked);

    entryActionLayout->addWidget(completeEntryBtn);
    entryActionLayout->addWidget(deleteEntryBtn);
    scLayout->addLayout(entryActionLayout);

    mainLayout->addWidget(schedCard);
}

void ScheduleRecoveryView::refreshData()
{
    populateRiskTable();
    populateScheduleTable(m_datePicker->date());
}

void ScheduleRecoveryView::populateRiskTable()
{
    QVector<DeadlineRisk> risks = DeadlineAnalyzer::instance().analyzeAllTasks();
    m_riskTable->setRowCount(risks.size());

    for (int r = 0; r < risks.size(); ++r) {
        const auto& risk = risks[r];

        m_riskTable->setItem(r, 0, new QTableWidgetItem(risk.taskTitle()));
        m_riskTable->setItem(r, 1, new QTableWidgetItem(risk.subject()));

        auto* prioItem = new QTableWidgetItem(Task::priorityToString(risk.priority()));
        prioItem->setForeground(Task::priorityColor(risk.priority()));
        prioItem->setFont(QFont("Segoe UI", 9, QFont::DemiBold));
        m_riskTable->setItem(r, 2, prioItem);

        m_riskTable->setItem(r, 3, new QTableWidgetItem(risk.deadline().toString("yyyy-MM-dd HH:mm")));

        auto* levelItem = new QTableWidgetItem(DeadlineRisk::levelToString(risk.level()));
        levelItem->setForeground(DeadlineRisk::levelColor(risk.level()));
        levelItem->setFont(QFont("Segoe UI", 9, QFont::Bold));
        m_riskTable->setItem(r, 4, levelItem);

        QString detail = QString("%1 %2").arg(risk.explanation(), risk.recommendation());
        auto* detailItem = new QTableWidgetItem(detail);
        detailItem->setToolTip(detail);
        m_riskTable->setItem(r, 5, detailItem);
    }
}

void ScheduleRecoveryView::populateScheduleTable(const QDate& date)
{
    QVector<ScheduleEntry> entries = DatabaseManager::instance().getScheduleEntriesForDate(date);
    m_scheduleTable->setRowCount(entries.size());

    double totalHours = 0.0;
    for (int r = 0; r < entries.size(); ++r) {
        const auto& e = entries[r];
        totalHours += e.allocatedHours();

        auto* titleItem = new QTableWidgetItem(e.taskTitle());
        titleItem->setData(Qt::UserRole, e.id());
        m_scheduleTable->setItem(r, 0, titleItem);

        m_scheduleTable->setItem(r, 1, new QTableWidgetItem(e.subject()));

        auto* prioItem = new QTableWidgetItem(Task::priorityToString(e.priority()));
        prioItem->setForeground(Task::priorityColor(e.priority()));
        m_scheduleTable->setItem(r, 2, prioItem);

        auto* hoursItem = new QTableWidgetItem(QString("%1h").arg(e.allocatedHours(), 0, 'f', 1));
        hoursItem->setTextAlignment(Qt::AlignCenter);
        m_scheduleTable->setItem(r, 3, hoursItem);

        auto* statusItem = new QTableWidgetItem(e.isCompleted() ? "Completed" : "Scheduled");
        statusItem->setForeground(e.isCompleted() ? QColor("#10B981") : QColor("#4F46E5"));
        m_scheduleTable->setItem(r, 4, statusItem);

        m_scheduleTable->setItem(r, 5, new QTableWidgetItem(e.notes()));
    }

    double cap = DeadlineAnalyzer::instance().getDailyCapacity(date);
    QString statusTxt = QString("Total Scheduled: %1h / Limit %2h")
                            .arg(QString::number(totalHours, 'f', 1))
                            .arg(QString::number(cap, 'f', 1));
    if (totalHours > cap) {
        statusTxt += " (⚠️ Overloaded)";
        m_dayWorkloadLabel->setStyleSheet("font-weight: bold; color: #DC2626;");
    } else {
        m_dayWorkloadLabel->setStyleSheet("font-weight: 600; color: #4F46E5;");
    }
    m_dayWorkloadLabel->setText(statusTxt);
}

void ScheduleRecoveryView::onScheduleDateChanged(const QDate& date)
{
    populateScheduleTable(date);
}

void ScheduleRecoveryView::onRunRecoveryAssistantClicked()
{
    RecoveryPlan plan = RecoveryScheduler::instance().generateRecoveryPlan();

    if (plan.isEmpty()) {
        QMessageBox::information(this, "Schedule Recovery", plan.summary());
        return;
    }

    RecoveryPlanDialog dlg(plan, this);
    if (dlg.exec() == QDialog::Accepted) {
        RecoveryScheduler::instance().applyRecoveryPlan(plan);
        refreshData();
        emit dataChanged();
        QMessageBox::information(this, "Schedule Applied", "Revised recovery schedule successfully committed to calendar!");
    }
}

void ScheduleRecoveryView::onAutoBalanceClicked()
{
    QString report;
    WorkloadBalancer::instance().balanceOverloadedDays(report);
    refreshData();
    emit dataChanged();
    QMessageBox::information(this, "Workload Leveling Result", report);
}

void ScheduleRecoveryView::onMarkEntryCompletedClicked()
{
    int row = m_scheduleTable->currentRow();
    if (row < 0 || row >= m_scheduleTable->rowCount()) {
        QMessageBox::information(this, "Select Entry", "Please select a scheduled slot to mark as completed.");
        return;
    }

    int entryId = m_scheduleTable->item(row, 0)->data(Qt::UserRole).toInt();
    QVector<ScheduleEntry> entries = DatabaseManager::instance().getScheduleEntriesForDate(m_datePicker->date());
    for (auto& e : entries) {
        if (e.id() == entryId) {
            e.setIsCompleted(true);
            DatabaseManager::instance().updateScheduleEntry(e);
            break;
        }
    }
    populateScheduleTable(m_datePicker->date());
    emit dataChanged();
}

void ScheduleRecoveryView::onDeleteScheduleEntryClicked()
{
    int row = m_scheduleTable->currentRow();
    if (row < 0 || row >= m_scheduleTable->rowCount()) {
        QMessageBox::information(this, "Select Entry", "Please select a scheduled slot to remove.");
        return;
    }

    int entryId = m_scheduleTable->item(row, 0)->data(Qt::UserRole).toInt();
    auto reply = QMessageBox::question(this, "Remove Scheduled Slot", "Are you sure you want to remove this study block from your calendar?", QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        DatabaseManager::instance().deleteScheduleEntry(entryId);
        populateScheduleTable(m_datePicker->date());
        emit dataChanged();
    }
}
