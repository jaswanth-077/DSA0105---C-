#include "RecoveryPlanDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QFrame>

RecoveryPlanDialog::RecoveryPlanDialog(const RecoveryPlan& plan, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Proposed Recovery Schedule Plan");
    resize(760, 520);
    setModal(true);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(16);

    auto* header = new QLabel("Review Proposed Schedule Adjustments", this);
    header->setStyleSheet("font-size: 18px; font-weight: bold; color: #0F172A;");
    layout->addWidget(header);

    // Summary Card
    auto* summaryCard = new QFrame(this);
    summaryCard->setStyleSheet("background-color: #F8FAFC; border: 1px solid #E2E8F0; border-radius: 8px; padding: 12px;");
    auto* cardLayout = new QVBoxLayout(summaryCard);
    cardLayout->setSpacing(8);

    auto* topStats = new QHBoxLayout();
    m_scoreLabel = new QLabel(QString("Feasibility: %1%").arg(plan.feasibilityScore(), 0, 'f', 0), summaryCard);
    m_scoreLabel->setStyleSheet("font-weight: 700; color: #4F46E5; font-size: 14px;");

    auto* conflicts = new QLabel(QString("Conflicts Resolved: %1").arg(plan.conflictsResolved()), summaryCard);
    conflicts->setStyleSheet("font-weight: 600; color: #10B981; font-size: 13px;");

    topStats->addWidget(m_scoreLabel);
    topStats->addWidget(conflicts);
    topStats->addStretch();
    cardLayout->addLayout(topStats);

    m_summaryLabel = new QLabel(plan.summary(), summaryCard);
    m_summaryLabel->setWordWrap(true);
    m_summaryLabel->setStyleSheet("color: #475569; font-size: 13px;");
    cardLayout->addWidget(m_summaryLabel);

    layout->addWidget(summaryCard);

    // Table of actions
    m_table = new QTableWidget(this);
    m_table->setColumnCount(6);
    m_table->setHorizontalHeaderLabels({"Task", "Priority", "Adjustment", "Original Date", "Proposed Date", "Workload"});
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->setVisible(false);

    populateTable(plan);
    layout->addWidget(m_table);

    // Bottom Action Buttons
    auto* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    auto* cancelBtn = new QPushButton("Cancel", this);
    cancelBtn->setProperty("class", "secondaryButton");
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    auto* applyBtn = new QPushButton("Apply Revised Schedule", this);
    applyBtn->setProperty("class", "primaryButton");
    connect(applyBtn, &QPushButton::clicked, this, &QDialog::accept);

    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(applyBtn);
    layout->addLayout(btnLayout);
}

void RecoveryPlanDialog::populateTable(const RecoveryPlan& plan)
{
    const auto& actions = plan.actions();
    m_table->setRowCount(actions.size());

    for (int r = 0; r < actions.size(); ++r) {
        const auto& a = actions[r];

        auto* taskItem = new QTableWidgetItem(a.taskTitle);
        taskItem->setToolTip(a.reason);
        m_table->setItem(r, 0, taskItem);

        auto* prioItem = new QTableWidgetItem(Task::priorityToString(a.priority));
        prioItem->setForeground(Task::priorityColor(a.priority));
        m_table->setItem(r, 1, prioItem);

        QString moveType = a.isNewAllocation ? "New Study Slot" : "Deferred / Shifted";
        auto* moveItem = new QTableWidgetItem(moveType);
        moveItem->setForeground(a.isNewAllocation ? QColor("#4F46E5") : QColor("#D97706"));
        m_table->setItem(r, 2, moveItem);

        QString origStr = a.originalDate.isValid() ? a.originalDate.toString("MMM d, ddd") : "—";
        m_table->setItem(r, 3, new QTableWidgetItem(origStr));

        QString propStr = a.proposedDate.isValid() ? a.proposedDate.toString("MMM d, ddd") : "—";
        auto* propItem = new QTableWidgetItem(propStr);
        propItem->setFont(QFont("Segoe UI", 9, QFont::Bold));
        m_table->setItem(r, 4, propItem);

        auto* hrsItem = new QTableWidgetItem(QString("%1h").arg(a.allocatedHours, 0, 'f', 1));
        hrsItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(r, 5, hrsItem);
    }
}
