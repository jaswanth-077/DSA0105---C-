#include "AnalyticsView.h"
#include "widgets/StatCard.h"
#include "widgets/WorkloadChartWidget.h"
#include "widgets/SubjectProgressWidget.h"
#include "../../services/WorkloadBalancer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QHeaderView>
#include <QMessageBox>

AnalyticsView::AnalyticsView(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
    refreshData();
}

void AnalyticsView::setupUi()
{
    auto* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("background-color: transparent;");

    auto* content = new QWidget(scrollArea);
    auto* mainLayout = new QVBoxLayout(content);
    mainLayout->setContentsMargins(30, 25, 30, 30);
    mainLayout->setSpacing(20);

    // Header
    auto* headerLayout = new QHBoxLayout();
    auto* titleContainer = new QWidget(content);
    auto* titleLayout = new QVBoxLayout(titleContainer);
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(4);

    auto* title = new QLabel("Workload & Performance Analytics", titleContainer);
    title->setProperty("class", "pageTitle");
    auto* subtitle = new QLabel("Analyze scheduled study velocity, capacity headroom, and subject-level progress.", titleContainer);
    subtitle->setProperty("class", "pageSubtitle");

    titleLayout->addWidget(title);
    titleLayout->addWidget(subtitle);
    headerLayout->addWidget(titleContainer);
    headerLayout->addStretch();

    m_rangeCombo = new QComboBox(content);
    m_rangeCombo->addItem("Next 7 Days", 7);
    m_rangeCombo->addItem("Next 14 Days", 14);
    m_rangeCombo->addItem("Next 30 Days", 30);
    m_rangeCombo->setCurrentIndex(1);
    connect(m_rangeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AnalyticsView::onRangeChanged);
    headerLayout->addWidget(m_rangeCombo);

    auto* balanceBtn = new QPushButton("⚖️ Balance Workload", content);
    balanceBtn->setProperty("class", "secondaryButton");
    connect(balanceBtn, &QPushButton::clicked, this, &AnalyticsView::onBalanceClicked);
    headerLayout->addWidget(balanceBtn);

    mainLayout->addLayout(headerLayout);

    // Stat cards
    auto* statsGrid = new QGridLayout();
    statsGrid->setSpacing(14);

    m_scheduledCard = new StatCard("📅", "Scheduled Study", "0.0h", "Within selected range", "#4F46E5", content);
    m_averageLoadCard = new StatCard("📊", "Average Daily Load", "0.0h", "Against daily capacity", "#6366F1", content);
    m_overloadedCard = new StatCard("⚠️", "Overloaded Days", "0", "Above capacity limit", "#EF4444", content);
    m_estimateCard = new StatCard("🧮", "Estimated vs Actual", "0%", "Logged of planned hours", "#10B981", content);

    statsGrid->addWidget(m_scheduledCard, 0, 0);
    statsGrid->addWidget(m_averageLoadCard, 0, 1);
    statsGrid->addWidget(m_overloadedCard, 0, 2);
    statsGrid->addWidget(m_estimateCard, 0, 3);
    mainLayout->addLayout(statsGrid);

    // Chart card
    auto* chartCard = new QFrame(content);
    chartCard->setProperty("class", "cardFrame");
    auto* chartLayout = new QVBoxLayout(chartCard);
    chartLayout->setContentsMargins(20, 18, 20, 18);
    chartLayout->setSpacing(12);

    auto* chartHeader = new QLabel("Daily Study Load vs Capacity Threshold", chartCard);
    chartHeader->setProperty("class", "sectionTitle");
    m_chart = new WorkloadChartWidget(chartCard);
    chartLayout->addWidget(chartHeader);
    chartLayout->addWidget(m_chart);
    mainLayout->addWidget(chartCard);

    // Subject progress + recommendations side by side
    auto* bottomRow = new QHBoxLayout();
    bottomRow->setSpacing(20);

    auto* subjectCard = new QFrame(content);
    subjectCard->setProperty("class", "cardFrame");
    auto* subjectLayout = new QVBoxLayout(subjectCard);
    subjectLayout->setContentsMargins(20, 18, 20, 18);
    subjectLayout->setSpacing(14);

    auto* subjectHeader = new QLabel("Subject Progress Breakdown", subjectCard);
    subjectHeader->setProperty("class", "sectionTitle");
    m_subjectProgress = new SubjectProgressWidget(subjectCard);
    subjectLayout->addWidget(subjectHeader);
    subjectLayout->addWidget(m_subjectProgress);
    subjectLayout->addStretch();

    auto* recoCard = new QFrame(content);
    recoCard->setProperty("class", "cardFrame");
    auto* recoLayout = new QVBoxLayout(recoCard);
    recoLayout->setContentsMargins(20, 18, 20, 18);
    recoLayout->setSpacing(12);

    auto* recoHeader = new QLabel("Proactive Balancing Recommendations", recoCard);
    recoHeader->setProperty("class", "sectionTitle");

    m_recommendationsTable = new QTableWidget(recoCard);
    m_recommendationsTable->setColumnCount(2);
    m_recommendationsTable->setHorizontalHeaderLabels({"Day", "Recommendation"});
    m_recommendationsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_recommendationsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_recommendationsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_recommendationsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_recommendationsTable->setWordWrap(true);
    m_recommendationsTable->verticalHeader()->setVisible(false);
    m_recommendationsTable->setMinimumHeight(200);

    recoLayout->addWidget(recoHeader);
    recoLayout->addWidget(m_recommendationsTable);

    bottomRow->addWidget(subjectCard, 2);
    bottomRow->addWidget(recoCard, 3);
    mainLayout->addLayout(bottomRow);

    scrollArea->setWidget(content);
    outerLayout->addWidget(scrollArea);
}

void AnalyticsView::refreshData()
{
    int dayCount = m_rangeCombo->currentData().toInt();
    if (dayCount <= 0) {
        dayCount = 14;
    }

    WorkloadAnalysis analysis = WorkloadBalancer::instance().analyzeWorkload(QDate::currentDate(), dayCount);

    m_scheduledCard->setValue(QString("%1h").arg(analysis.totalScheduledHours, 0, 'f', 1));
    m_scheduledCard->setSubtext(QString("%1-day planning horizon").arg(dayCount));

    m_averageLoadCard->setValue(QString("%1h").arg(analysis.averageDailyLoad, 0, 'f', 1));
    m_averageLoadCard->setSubtext(QString("Capacity %1h/day").arg(analysis.totalCapacityHours / dayCount, 0, 'f', 1));

    m_overloadedCard->setValue(QString::number(analysis.overloadedDaysCount));
    m_overloadedCard->setSubtext(analysis.overloadedDaysCount > 0 ? "Rebalance recommended" : "All days within limits");

    QMap<QString, double> estimateActual = WorkloadBalancer::instance().getEstimatedVsActualHours();
    double estimated = estimateActual.value("Estimated", 0.0);
    double actual = estimateActual.value("Actual", 0.0);
    double loggedPercent = estimated > 0.0 ? (actual / estimated) * 100.0 : 0.0;
    m_estimateCard->setValue(QString("%1%").arg(loggedPercent, 0, 'f', 0));
    m_estimateCard->setSubtext(QString("%1h logged of %2h planned")
                                   .arg(QString::number(actual, 'f', 1), QString::number(estimated, 'f', 1)));

    // Chart data
    QVector<ChartDayData> chartData;
    for (const auto& dw : analysis.dailyWorkloads) {
        ChartDayData cdd;
        cdd.date = dw.date;
        cdd.dayName = dw.date.toString("ddd");
        cdd.scheduledHours = dw.scheduledHours;
        cdd.capacityHours = dw.capacityHours;
        cdd.isOverloaded = dw.isOverloaded;
        chartData.append(cdd);
    }
    m_chart->setData(chartData);

    m_subjectProgress->setSubjects(WorkloadBalancer::instance().getSubjectProgressSummary());

    m_recommendationsTable->setRowCount(analysis.recommendations.size());
    for (int r = 0; r < analysis.recommendations.size(); ++r) {
        QString text = analysis.recommendations[r];
        auto* scopeItem = new QTableWidgetItem(analysis.overloadedDaysCount > 0 ? "Action" : "Status");
        scopeItem->setForeground(analysis.overloadedDaysCount > 0 ? QColor("#D97706") : QColor("#10B981"));
        m_recommendationsTable->setItem(r, 0, scopeItem);

        auto* textItem = new QTableWidgetItem(text);
        textItem->setToolTip(text);
        m_recommendationsTable->setItem(r, 1, textItem);
    }
    m_recommendationsTable->resizeRowsToContents();
}

void AnalyticsView::onRangeChanged()
{
    refreshData();
}

void AnalyticsView::onBalanceClicked()
{
    QString report;
    WorkloadBalancer::instance().balanceOverloadedDays(report);
    QMessageBox::information(this, "Workload Leveling Result", report);
    refreshData();
    emit dataChanged();
}
