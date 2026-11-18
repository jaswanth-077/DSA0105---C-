#pragma once

#include <QMainWindow>

class QStackedWidget;

class Sidebar;
class DashboardView;
class TaskManagerView;
class ScheduleRecoveryView;
class AnalyticsView;
class FocusTimerView;
class SettingsView;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onTabChanged(int index);
    void onNewTaskRequested();
    void refreshAllViews();

private:
    void setupUi();

    Sidebar* m_sidebar;
    QStackedWidget* m_stack;

    DashboardView* m_dashboard;
    TaskManagerView* m_taskManager;
    ScheduleRecoveryView* m_recovery;
    AnalyticsView* m_analytics;
    FocusTimerView* m_focusTimer;
    SettingsView* m_settings;
};
