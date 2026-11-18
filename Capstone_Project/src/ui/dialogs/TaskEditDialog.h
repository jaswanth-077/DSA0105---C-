#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QLabel>
#include "../../models/Task.h"

class TaskEditDialog : public QDialog {
    Q_OBJECT

public:
    explicit TaskEditDialog(QWidget* parent = nullptr, const Task* existingTask = nullptr);

    Task getTask() const;

private slots:
    void onSaveClicked();
    void onPercentageChanged(int val);

private:
    void populateSubjects();

    QLineEdit* m_titleEdit;
    QTextEdit* m_descriptionEdit;
    QComboBox* m_subjectCombo;
    QComboBox* m_typeCombo;
    QComboBox* m_priorityCombo;
    QDateTimeEdit* m_deadlineEdit;
    QDoubleSpinBox* m_estimatedDurationSpin;
    QDoubleSpinBox* m_remainingDurationSpin;
    QSpinBox* m_completionSpin;
    QComboBox* m_statusCombo;
    QLabel* m_errorLabel;

    int m_taskId;
    QDateTime m_createdAt;
};
