#include "LogSessionDialog.h"
#include "../../database/DatabaseManager.h"
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

LogSessionDialog::LogSessionDialog(QWidget* parent, int preselectedTaskId)
    : QDialog(parent)
{
    setWindowTitle("Log Study Session");
    resize(460, 380);
    setModal(true);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(16);

    auto* header = new QLabel("Record Completed Study Time", this);
    header->setStyleSheet("font-size: 17px; font-weight: bold; color: #0F172A;");
    layout->addWidget(header);

    auto* formLayout = new QFormLayout();
    formLayout->setSpacing(12);

    m_taskCombo = new QComboBox(this);
    populateTasks(preselectedTaskId);
    formLayout->addRow("Associated Task:", m_taskCombo);

    m_dateEdit = new QDateTimeEdit(QDateTime::currentDateTime(), this);
    m_dateEdit->setCalendarPopup(true);
    m_dateEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
    formLayout->addRow("Session Timestamp:", m_dateEdit);

    m_durationSpin = new QSpinBox(this);
    m_durationSpin->setRange(5, 720);
    m_durationSpin->setSingleStep(15);
    m_durationSpin->setValue(45);
    m_durationSpin->setSuffix(" minutes");
    formLayout->addRow("Study Duration:", m_durationSpin);

    // Quick duration pills
    auto* quickBtnLayout = new QHBoxLayout();
    QVector<int> presets = {25, 45, 60, 90, 120};
    for (int p : presets) {
        auto* btn = new QPushButton(QString("%1m").arg(p), this);
        btn->setProperty("class", "secondaryButton");
        connect(btn, &QPushButton::clicked, this, [this, p]() {
            m_durationSpin->setValue(p);
        });
        quickBtnLayout->addWidget(btn);
    }
    formLayout->addRow("", quickBtnLayout);

    m_notesEdit = new QTextEdit(this);
    m_notesEdit->setPlaceholderText("Notes, topics revised, or breakthroughs during this session...");
    m_notesEdit->setMaximumHeight(80);
    formLayout->addRow("Session Notes:", m_notesEdit);

    layout->addLayout(formLayout);

    auto* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    auto* cancelBtn = new QPushButton("Cancel", this);
    cancelBtn->setProperty("class", "secondaryButton");
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    auto* saveBtn = new QPushButton("Save Session", this);
    saveBtn->setProperty("class", "primaryButton");
    connect(saveBtn, &QPushButton::clicked, this, &QDialog::accept);

    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(saveBtn);
    layout->addLayout(btnLayout);
}

void LogSessionDialog::populateTasks(int preselectedId)
{
    m_taskCombo->clear();
    QVector<Task> activeTasks = DatabaseManager::instance().getActiveTasks();
    int selectIndex = 0;
    for (int i = 0; i < activeTasks.size(); ++i) {
        const auto& t = activeTasks[i];
        m_taskCombo->addItem(QString("%1 (%2)").arg(t.title(), t.subject()), t.id());
        if (t.id() == preselectedId) {
            selectIndex = i;
        }
    }
    if (!activeTasks.isEmpty()) {
        m_taskCombo->setCurrentIndex(selectIndex);
    }
}

int LogSessionDialog::getSelectedTaskId() const
{
    return m_taskCombo->currentData().toInt();
}

int LogSessionDialog::getLoggedMinutes() const
{
    return m_durationSpin->value();
}

StudySession LogSessionDialog::getStudySession() const
{
    return StudySession(
        -1,
        getSelectedTaskId(),
        m_taskCombo->currentText(),
        m_dateEdit->dateTime(),
        m_durationSpin->value(),
        m_notesEdit->toPlainText().trimmed()
    );
}
