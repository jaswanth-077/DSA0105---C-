#pragma once

#include <QDialog>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QSpinBox>
#include <QTextEdit>
#include "../../models/StudySession.h"

class LogSessionDialog : public QDialog {
    Q_OBJECT

public:
    explicit LogSessionDialog(QWidget* parent = nullptr, int preselectedTaskId = -1);

    StudySession getStudySession() const;
    int getSelectedTaskId() const;
    int getLoggedMinutes() const;

private:
    void populateTasks(int preselectedId);

    QComboBox* m_taskCombo;
    QDateTimeEdit* m_dateEdit;
    QSpinBox* m_durationSpin;
    QTextEdit* m_notesEdit;
};
