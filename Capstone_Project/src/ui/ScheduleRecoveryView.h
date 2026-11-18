#pragma once

#include <QWidget>
#include <QLabel>
#include <QTableWidget>
#include <QDateEdit>
#include <QPushButton>

class ScheduleRecoveryView : public QWidget {
    Q_OBJECT

public:
    explicit ScheduleRecoveryView(QWidget* parent = nullptr);

    void refreshData();

signals:
    void dataChanged();

private slots:
    void onRunRecoveryAssistantClicked();
    void onAutoBalanceClicked();
    void onScheduleDateChanged(const QDate& date);
    void onMarkEntryCompletedClicked();
    void onDeleteScheduleEntryClicked();

private:
    void setupUi();
    void populateRiskTable();
    void populateScheduleTable(const QDate& date);

    // Risk Overview Table
    QTableWidget* m_riskTable;

    // Daily Schedule Table
    QDateEdit* m_datePicker;
    QLabel* m_dayWorkloadLabel;
    QTableWidget* m_scheduleTable;

    QPushButton* m_recoveryBtn;
    QPushButton* m_balanceBtn;
};
