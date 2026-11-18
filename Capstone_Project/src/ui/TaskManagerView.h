#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QTableWidget>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include "../../models/Task.h"

class TaskManagerView : public QWidget {
    Q_OBJECT

public:
    explicit TaskManagerView(QWidget* parent = nullptr);

    void refreshTasks();
    void selectTaskById(int taskId);

signals:
    void dataChanged();

public slots:
    void onNewTaskClicked();

private slots:
    void onFilterChanged();
    void onTaskSelectionChanged();
    void onEditTaskClicked();
    void onDeleteTaskClicked();
    void onCompleteTaskClicked();
    void onLogSessionClicked();

private:
    void setupUi();
    void updateDetailPanel(const Task* task);

    // Filter controls
    QLineEdit* m_searchEdit;
    QComboBox* m_subjectFilter;
    QComboBox* m_priorityFilter;
    QComboBox* m_statusFilter;
    QComboBox* m_sortCombo;

    // Table
    QTableWidget* m_table;

    // Detail Panel
    QWidget* m_detailPanel;
    QLabel* m_detailTitle;
    QLabel* m_detailSubject;
    QLabel* m_detailType;
    QLabel* m_detailPriority;
    QLabel* m_detailDeadline;
    QLabel* m_detailHours;
    QLabel* m_detailProgress;
    QLabel* m_detailStatus;
    QTextEdit* m_detailDescription;
    QLabel* m_detailRiskExplanation;

    QPushButton* m_editBtn;
    QPushButton* m_logSessionBtn;
    QPushButton* m_completeBtn;
    QPushButton* m_deleteBtn;

    int m_selectedTaskId;
};
