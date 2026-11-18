#include "FocusTimerView.h"
#include "../../database/DatabaseManager.h"
#include "../../services/TaskService.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include <algorithm>

FocusTimerView::FocusTimerView(QWidget* parent)
    : QWidget(parent)
    , m_remainingSeconds(0)
    , m_totalSeconds(0)
    , m_completedSessions(0)
    , m_focusPhase(true)
    , m_running(false)
{
    setupUi();
    refreshData();

    m_timer = new QTimer(this);
    m_timer->setInterval(1000);
    connect(m_timer, &QTimer::timeout, this, &FocusTimerView::onTick);

    startPhase(true);
    m_running = false;
    m_timer->stop();
    m_startPauseBtn->setText("▶ Start Focus Session");
    updateDisplay();
}

void FocusTimerView::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 25, 30, 30);
    mainLayout->setSpacing(20);

    // Header
    auto* titleContainer = new QWidget(this);
    auto* titleLayout = new QVBoxLayout(titleContainer);
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(4);

    auto* title = new QLabel("Focus Timer", titleContainer);
    title->setProperty("class", "pageTitle");
    auto* subtitle = new QLabel("Deep-work sessions that log themselves against the task you are studying.", titleContainer);
    subtitle->setProperty("class", "pageSubtitle");

    titleLayout->addWidget(title);
    titleLayout->addWidget(subtitle);
    mainLayout->addWidget(titleContainer);

    // Timer card
    auto* timerCard = new QFrame(this);
    timerCard->setProperty("class", "cardFrame");
    auto* timerLayout = new QVBoxLayout(timerCard);
    timerLayout->setContentsMargins(30, 26, 30, 26);
    timerLayout->setSpacing(16);

    m_phaseLabel = new QLabel("Focus Session", timerCard);
    m_phaseLabel->setAlignment(Qt::AlignCenter);
    m_phaseLabel->setStyleSheet("color: #4F46E5; font-size: 13px; font-weight: 700; letter-spacing: 1px;");

    m_timeLabel = new QLabel("25:00", timerCard);
    m_timeLabel->setAlignment(Qt::AlignCenter);
    m_timeLabel->setMinimumHeight(110);
    m_timeLabel->setStyleSheet("color: #0F172A; font-size: 68px; font-weight: 700;");

    m_progressBar = new QProgressBar(timerCard);
    m_progressBar->setRange(0, 1500);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(false);
    m_progressBar->setFixedHeight(10);

    timerLayout->addWidget(m_phaseLabel);
    timerLayout->addWidget(m_timeLabel);
    timerLayout->addWidget(m_progressBar);

    // Task selector
    auto* taskRow = new QHBoxLayout();
    auto* taskLabel = new QLabel("Studying:", timerCard);
    taskLabel->setStyleSheet("font-weight: 600; color: #475569;");
    m_taskCombo = new QComboBox(timerCard);
    m_taskCombo->setMinimumWidth(360);

    taskRow->addStretch();
    taskRow->addWidget(taskLabel);
    taskRow->addWidget(m_taskCombo);
    taskRow->addStretch();
    timerLayout->addLayout(taskRow);

    // Durations
    auto* durationRow = new QHBoxLayout();
    durationRow->setSpacing(18);

    auto* focusBox = new QWidget(timerCard);
    auto* focusLayout = new QVBoxLayout(focusBox);
    focusLayout->setContentsMargins(0, 0, 0, 0);
    focusLayout->setSpacing(6);
    auto* focusLabel = new QLabel("Focus length", focusBox);
    focusLabel->setStyleSheet("color: #64748B; font-size: 11px; font-weight: 600; text-transform: uppercase;");
    m_focusMinutesSpin = new QSpinBox(focusBox);
    m_focusMinutesSpin->setRange(5, 180);
    m_focusMinutesSpin->setSingleStep(5);
    m_focusMinutesSpin->setValue(25);
    m_focusMinutesSpin->setSuffix(" min");
    focusLayout->addWidget(focusLabel);
    focusLayout->addWidget(m_focusMinutesSpin);

    auto* breakBox = new QWidget(timerCard);
    auto* breakLayout = new QVBoxLayout(breakBox);
    breakLayout->setContentsMargins(0, 0, 0, 0);
    breakLayout->setSpacing(6);
    auto* breakLabel = new QLabel("Break length", breakBox);
    breakLabel->setStyleSheet("color: #64748B; font-size: 11px; font-weight: 600; text-transform: uppercase;");
    m_breakMinutesSpin = new QSpinBox(breakBox);
    m_breakMinutesSpin->setRange(0, 60);
    m_breakMinutesSpin->setSingleStep(1);
    m_breakMinutesSpin->setValue(5);
    m_breakMinutesSpin->setSuffix(" min");
    breakLayout->addWidget(breakLabel);
    breakLayout->addWidget(m_breakMinutesSpin);

    durationRow->addStretch();
    durationRow->addWidget(focusBox);
    durationRow->addWidget(breakBox);
    durationRow->addStretch();
    timerLayout->addLayout(durationRow);

    connect(m_focusMinutesSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int) {
        if (!m_running && m_focusPhase) {
            m_totalSeconds = m_focusMinutesSpin->value() * 60;
            m_remainingSeconds = m_totalSeconds;
            updateDisplay();
        }
    });

    // Controls
    auto* controlRow = new QHBoxLayout();
    controlRow->setSpacing(12);
    controlRow->addStretch();

    m_startPauseBtn = new QPushButton("▶ Start Focus Session", timerCard);
    m_startPauseBtn->setProperty("class", "primaryButton");
    connect(m_startPauseBtn, &QPushButton::clicked, this, &FocusTimerView::onStartPauseClicked);

    m_resetBtn = new QPushButton("Reset", timerCard);
    m_resetBtn->setProperty("class", "secondaryButton");
    connect(m_resetBtn, &QPushButton::clicked, this, &FocusTimerView::onResetClicked);

    controlRow->addWidget(m_startPauseBtn);
    controlRow->addWidget(m_resetBtn);
    controlRow->addStretch();
    timerLayout->addLayout(controlRow);

    m_sessionCountLabel = new QLabel("0 focus sessions completed this sitting", timerCard);
    m_sessionCountLabel->setAlignment(Qt::AlignCenter);
    m_sessionCountLabel->setStyleSheet("color: #94A3B8; font-size: 12px;");
    timerLayout->addWidget(m_sessionCountLabel);

    mainLayout->addWidget(timerCard);
    mainLayout->addStretch();
}

void FocusTimerView::refreshData()
{
    int previousId = m_taskCombo->currentData().toInt();

    m_taskCombo->blockSignals(true);
    m_taskCombo->clear();

    QVector<Task> activeTasks = TaskService::instance().getActiveTasks();
    for (const auto& t : activeTasks) {
        m_taskCombo->addItem(QString("%1 — %2").arg(t.title(), t.subject()), t.id());
    }

    if (activeTasks.isEmpty()) {
        m_taskCombo->addItem("No active tasks available", -1);
        m_taskCombo->setEnabled(false);
        m_startPauseBtn->setEnabled(false);
    } else {
        m_taskCombo->setEnabled(true);
        m_startPauseBtn->setEnabled(true);
        int idx = m_taskCombo->findData(previousId);
        if (idx >= 0) {
            m_taskCombo->setCurrentIndex(idx);
        }
    }

    m_taskCombo->blockSignals(false);
}

void FocusTimerView::startPhase(bool focusPhase)
{
    m_focusPhase = focusPhase;
    int minutes = focusPhase ? m_focusMinutesSpin->value() : m_breakMinutesSpin->value();
    m_totalSeconds = std::max(1, minutes * 60);
    m_remainingSeconds = m_totalSeconds;
    m_phaseLabel->setText(focusPhase ? "Focus Session" : "Break");
    m_phaseLabel->setStyleSheet(focusPhase
                                    ? "color: #4F46E5; font-size: 13px; font-weight: 700; letter-spacing: 1px;"
                                    : "color: #10B981; font-size: 13px; font-weight: 700; letter-spacing: 1px;");
    updateDisplay();
}

void FocusTimerView::onStartPauseClicked()
{
    if (m_running) {
        m_timer->stop();
        m_running = false;
        m_startPauseBtn->setText("▶ Resume");
        return;
    }

    if (m_remainingSeconds <= 0) {
        startPhase(true);
    }

    m_running = true;
    m_timer->start(1000);
    m_startPauseBtn->setText("⏸ Pause");
}

void FocusTimerView::onResetClicked()
{
    m_timer->stop();
    m_running = false;
    startPhase(true);
    m_startPauseBtn->setText("▶ Start Focus Session");
}

void FocusTimerView::onTick()
{
    if (m_remainingSeconds > 0) {
        --m_remainingSeconds;
    }

    updateDisplay();

    if (m_remainingSeconds <= 0) {
        finishPhase();
    }
}

void FocusTimerView::finishPhase()
{
    m_timer->stop();
    m_running = false;

    if (m_focusPhase) {
        logFocusSession();
        ++m_completedSessions;
        m_sessionCountLabel->setText(QString("%1 focus session(s) completed this sitting").arg(m_completedSessions));

        if (m_breakMinutesSpin->value() > 0) {
            startPhase(false);
            m_running = true;
            m_timer->start(1000);
            m_startPauseBtn->setText("⏸ Pause");
            return;
        }
    } else {
        startPhase(true);
        m_startPauseBtn->setText("▶ Start Focus Session");
        return;
    }

    startPhase(true);
    m_startPauseBtn->setText("▶ Start Focus Session");
}

void FocusTimerView::logFocusSession()
{
    int taskId = m_taskCombo->currentData().toInt();
    if (taskId < 0) {
        return;
    }

    int minutes = m_focusMinutesSpin->value();

    StudySession session(
        -1,
        taskId,
        m_taskCombo->currentText(),
        QDateTime::currentDateTime(),
        minutes,
        "Focus timer session"
    );
    DatabaseManager::instance().addStudySession(session);

    auto opt = TaskService::instance().getTask(taskId);
    if (opt) {
        Task t = *opt;
        double loggedHours = static_cast<double>(minutes) / 60.0;
        double newRemaining = std::max(0.0, t.remainingDuration() - loggedHours);
        t.setRemainingDuration(newRemaining);

        if (t.estimatedDuration() > 0.0) {
            int newPct = std::min(100, static_cast<int>(((t.estimatedDuration() - newRemaining) / t.estimatedDuration()) * 100.0));
            t.setCompletionPercentage(newPct);
        }

        QString error;
        TaskService::instance().updateTask(t, error);
    }

    emit dataChanged();
}

void FocusTimerView::updateDisplay()
{
    int minutes = m_remainingSeconds / 60;
    int seconds = m_remainingSeconds % 60;
    m_timeLabel->setText(QString("%1:%2")
                             .arg(minutes, 2, 10, QChar('0'))
                             .arg(seconds, 2, 10, QChar('0')));

    m_progressBar->setRange(0, std::max(1, m_totalSeconds));
    m_progressBar->setValue(std::max(0, m_totalSeconds - m_remainingSeconds));
}
