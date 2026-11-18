#pragma once

#include <QWidget>
#include <QComboBox>
#include <QTableWidget>

class StatCard;
class WorkloadChartWidget;
class SubjectProgressWidget;

class AnalyticsView : public QWidget {
    Q_OBJECT

public:
    explicit AnalyticsView(QWidget* parent = nullptr);

    void refreshData();

signals:
    void dataChanged();

private slots:
    void onRangeChanged();
    void onBalanceClicked();

private:
    void setupUi();

    QComboBox* m_rangeCombo;
    StatCard* m_scheduledCard;
    StatCard* m_averageLoadCard;
    StatCard* m_overloadedCard;
    StatCard* m_estimateCard;
    WorkloadChartWidget* m_chart;
    SubjectProgressWidget* m_subjectProgress;
    QTableWidget* m_recommendationsTable;
};
