#pragma once

#include <QDialog>
#include <QTableWidget>
#include <QLabel>
#include "../../models/RecoveryPlan.h"

class RecoveryPlanDialog : public QDialog {
    Q_OBJECT

public:
    explicit RecoveryPlanDialog(const RecoveryPlan& plan, QWidget* parent = nullptr);

private:
    void populateTable(const RecoveryPlan& plan);

    QLabel* m_summaryLabel;
    QLabel* m_scoreLabel;
    QTableWidget* m_table;
};
