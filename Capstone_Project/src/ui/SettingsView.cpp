#include "SettingsView.h"
#include "../../database/DatabaseManager.h"
#include "../../services/DeadlineAnalyzer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QFrame>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>

SettingsView::SettingsView(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
    refreshData();
}

void SettingsView::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 25, 30, 30);
    mainLayout->setSpacing(20);

    // Header
    auto* titleContainer = new QWidget(this);
    auto* titleLayout = new QVBoxLayout(titleContainer);
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(4);

    auto* title = new QLabel("Settings & Preferences", titleContainer);
    title->setProperty("class", "pageTitle");
    auto* subtitle = new QLabel("Tune your daily study capacity and manage locally stored academic data.", titleContainer);
    subtitle->setProperty("class", "pageSubtitle");

    titleLayout->addWidget(title);
    titleLayout->addWidget(subtitle);
    mainLayout->addWidget(titleContainer);

    // Capacity card
    auto* capacityCard = new QFrame(this);
    capacityCard->setProperty("class", "cardFrame");
    auto* capacityLayout = new QVBoxLayout(capacityCard);
    capacityLayout->setContentsMargins(22, 20, 22, 20);
    capacityLayout->setSpacing(14);

    auto* capacityHeader = new QLabel("Daily Study Capacity", capacityCard);
    capacityHeader->setProperty("class", "sectionTitle");
    capacityLayout->addWidget(capacityHeader);

    auto* capacityHint = new QLabel(
        "The deadline risk engine and recovery scheduler never plan more than these limits per day. "
        "Weekend capacity can differ from weekdays.", capacityCard);
    capacityHint->setWordWrap(true);
    capacityHint->setStyleSheet("color: #64748B; font-size: 12px;");
    capacityLayout->addWidget(capacityHint);

    auto* formLayout = new QFormLayout();
    formLayout->setSpacing(12);
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_weekdayCapacitySpin = new QDoubleSpinBox(capacityCard);
    m_weekdayCapacitySpin->setRange(0.5, 16.0);
    m_weekdayCapacitySpin->setSingleStep(0.5);
    m_weekdayCapacitySpin->setSuffix(" hours");
    formLayout->addRow("Weekday limit:", m_weekdayCapacitySpin);

    m_weekendCapacitySpin = new QDoubleSpinBox(capacityCard);
    m_weekendCapacitySpin->setRange(0.5, 16.0);
    m_weekendCapacitySpin->setSingleStep(0.5);
    m_weekendCapacitySpin->setSuffix(" hours");
    formLayout->addRow("Weekend limit:", m_weekendCapacitySpin);

    m_autoBalanceCheck = new QCheckBox("Suggest automatic workload balancing when a day is overloaded", capacityCard);
    formLayout->addRow("", m_autoBalanceCheck);

    capacityLayout->addLayout(formLayout);

    auto* saveRow = new QHBoxLayout();
    saveRow->addStretch();
    auto* saveBtn = new QPushButton("Save Preferences", capacityCard);
    saveBtn->setProperty("class", "primaryButton");
    connect(saveBtn, &QPushButton::clicked, this, &SettingsView::onSavePreferencesClicked);
    saveRow->addWidget(saveBtn);
    capacityLayout->addLayout(saveRow);

    m_savedLabel = new QLabel("", capacityCard);
    m_savedLabel->setStyleSheet("color: #10B981; font-size: 12px; font-weight: 600;");
    m_savedLabel->hide();
    capacityLayout->addWidget(m_savedLabel);

    mainLayout->addWidget(capacityCard);

    // Storage card
    auto* storageCard = new QFrame(this);
    storageCard->setProperty("class", "cardFrame");
    auto* storageLayout = new QVBoxLayout(storageCard);
    storageLayout->setContentsMargins(22, 20, 22, 20);
    storageLayout->setSpacing(12);

    auto* storageHeader = new QLabel("Data & Storage", storageCard);
    storageHeader->setProperty("class", "sectionTitle");
    storageLayout->addWidget(storageHeader);

    m_storagePathLabel = new QLabel("", storageCard);
    m_storagePathLabel->setWordWrap(true);
    m_storagePathLabel->setStyleSheet("color: #64748B; font-size: 12px;");
    m_storagePathLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    storageLayout->addWidget(m_storagePathLabel);

    auto* storageHint = new QLabel(
        "All tasks, schedules, and study sessions are persisted in a local SQLite database. "
        "Data survives application restarts.", storageCard);
    storageHint->setWordWrap(true);
    storageHint->setStyleSheet("color: #94A3B8; font-size: 12px;");
    storageLayout->addWidget(storageHint);

    auto* actionRow = new QHBoxLayout();
    actionRow->setSpacing(12);

    auto* seedBtn = new QPushButton("Restore Sample Data", storageCard);
    seedBtn->setProperty("class", "secondaryButton");
    connect(seedBtn, &QPushButton::clicked, this, &SettingsView::onRestoreSampleDataClicked);

    auto* clearBtn = new QPushButton("Delete All Tasks", storageCard);
    clearBtn->setProperty("class", "dangerButton");
    connect(clearBtn, &QPushButton::clicked, this, &SettingsView::onClearAllDataClicked);

    actionRow->addStretch();
    actionRow->addWidget(seedBtn);
    actionRow->addWidget(clearBtn);
    storageLayout->addLayout(actionRow);

    mainLayout->addWidget(storageCard);
    mainLayout->addStretch();
}

void SettingsView::refreshData()
{
    m_weekdayCapacitySpin->setValue(
        DatabaseManager::instance().getPreference("weekday_capacity_hours", "5.0").toDouble());
    m_weekendCapacitySpin->setValue(
        DatabaseManager::instance().getPreference("weekend_capacity_hours", "6.5").toDouble());
    m_autoBalanceCheck->setChecked(
        DatabaseManager::instance().getPreference("auto_balance", "true") == "true");

    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dataDir.isEmpty()) {
        dataDir = "./data";
    }
    QString dbPath = QDir(dataDir).filePath("smart_study_planner.db");
    m_storagePathLabel->setText(QString("Database file: %1").arg(QDir::toNativeSeparators(dbPath)));
    m_savedLabel->hide();
}

void SettingsView::onSavePreferencesClicked()
{
    double weekday = m_weekdayCapacitySpin->value();
    double weekend = m_weekendCapacitySpin->value();

    if (weekday <= 0.0 || weekend <= 0.0) {
        QMessageBox::warning(this, "Invalid Capacity", "Daily capacity limits must be greater than zero hours.");
        return;
    }

    DatabaseManager::instance().setPreference("weekday_capacity_hours", QString::number(weekday));
    DatabaseManager::instance().setPreference("weekend_capacity_hours", QString::number(weekend));
    DatabaseManager::instance().setPreference("auto_balance", m_autoBalanceCheck->isChecked() ? "true" : "false");

    m_savedLabel->setText(QString("Preferences saved. Today's capacity is now %1h.")
                              .arg(QString::number(DeadlineAnalyzer::instance().getDailyCapacity(QDate::currentDate()), 'f', 1)));
    m_savedLabel->show();

    emit dataChanged();
}

void SettingsView::onRestoreSampleDataClicked()
{
    auto reply = QMessageBox::question(
        this,
        "Restore Sample Data",
        "This will replace all current tasks, schedule entries, and study sessions with the sample semester dataset.\n\nContinue?",
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        return;
    }

    DatabaseManager::instance().seedRealisticData();
    refreshData();
    emit dataChanged();
    QMessageBox::information(this, "Sample Data Restored", "The starter academic dataset has been loaded.");
}

void SettingsView::onClearAllDataClicked()
{
    auto reply = QMessageBox::warning(
        this,
        "Delete All Data",
        "This permanently deletes every task, scheduled study block, and logged session.\n\nThis cannot be undone. Continue?",
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        return;
    }

    DatabaseManager::instance().clearAllData();
    refreshData();
    emit dataChanged();
    QMessageBox::information(this, "Data Cleared", "All academic data has been deleted from the local database.");
}
