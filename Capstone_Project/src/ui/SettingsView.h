#pragma once

#include <QWidget>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLabel>

class SettingsView : public QWidget {
    Q_OBJECT

public:
    explicit SettingsView(QWidget* parent = nullptr);

    void refreshData();

signals:
    void dataChanged();

private slots:
    void onSavePreferencesClicked();
    void onRestoreSampleDataClicked();
    void onClearAllDataClicked();

private:
    void setupUi();

    QDoubleSpinBox* m_weekdayCapacitySpin;
    QDoubleSpinBox* m_weekendCapacitySpin;
    QCheckBox* m_autoBalanceCheck;
    QLabel* m_storagePathLabel;
    QLabel* m_savedLabel;
};
