#pragma once

#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QTimer>

class FocusTimerView : public QWidget {
    Q_OBJECT

public:
    explicit FocusTimerView(QWidget* parent = nullptr);

    void refreshData();

signals:
    void dataChanged();

private slots:
    void onStartPauseClicked();
    void onResetClicked();
    void onTick();

private:
    void setupUi();
    void startPhase(bool focusPhase);
    void finishPhase();
    void updateDisplay();
    void logFocusSession();

    QComboBox* m_taskCombo;
    QSpinBox* m_focusMinutesSpin;
    QSpinBox* m_breakMinutesSpin;
    QLabel* m_timeLabel;
    QLabel* m_phaseLabel;
    QLabel* m_sessionCountLabel;
    QProgressBar* m_progressBar;
    QPushButton* m_startPauseBtn;
    QPushButton* m_resetBtn;

    QTimer* m_timer;
    int m_remainingSeconds;
    int m_totalSeconds;
    int m_completedSessions;
    bool m_focusPhase;
    bool m_running;
};
