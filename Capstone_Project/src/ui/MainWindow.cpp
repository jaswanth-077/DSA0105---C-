#include "MainWindow.h"
#include "Sidebar.h"
#include "DashboardView.h"
#include "TaskManagerView.h"
#include "ScheduleRecoveryView.h"
#include "AnalyticsView.h"
#include "FocusTimerView.h"
#include "SettingsView.h"
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QStatusBar>
#include <QLabel>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setupUi();
}

void MainWindow::setupUi()
{
    setWindowTitle("Smart Study Planner — Academic Workload Intelligence");
    resize(1360, 860);
    setMinimumSize(1120, 720);

    auto* central = new QWidget(this);
    central->setObjectName("centralWidget");

    auto* layout = new QHBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_sidebar = new Sidebar(central);
    layout->addWidget(m_sidebar);

    m_stack = new QStackedWidget(central);
    m_dashboard = new DashboardView(m_stack);
    m_taskManager = new TaskManagerView(m_stack);
    m_recovery = new ScheduleRecoveryView(m_stack);
    m_analytics = new AnalyticsView(m_stack);
    m_focusTimer = new FocusTimerView(m_stack);
    m_settings = new SettingsView(m_stack);

    m_stack->addWidget(m_dashboard);
    m_stack->addWidget(m_taskManager);
    m_stack->addWidget(m_recovery);
    m_stack->addWidget(m_analytics);
    m_stack->addWidget(m_focusTimer);
    m_stack->addWidget(m_settings);
    layout->addWidget(m_stack, 1);

    setCentralWidget(central);

    auto* storageLabel = new QLabel("Local SQLite storage • Offline ready", this);
    storageLabel->setStyleSheet("color: #94A3B8; font-size: 11px; padding-right: 12px;");
    statusBar()->addPermanentWidget(storageLabel);

    connect(m_sidebar, &Sidebar::tabChanged, this, &MainWindow::onTabChanged);
    connect(m_sidebar, &Sidebar::newTaskRequested, this, &MainWindow::onNewTaskRequested);

    connect(m_dashboard, &DashboardView::newTaskRequested, this, &MainWindow::onNewTaskRequested);
    connect(m_dashboard, &DashboardView::openRecoveryRequested, this, [this]() {
        m_sidebar->setCurrentIndex(2);
        onTabChanged(2);
    });
    connect(m_dashboard, &DashboardView::openTaskManagerRequested, this, [this]() {
        m_sidebar->setCurrentIndex(1);
        onTabChanged(1);
    });

    connect(m_taskManager, &TaskManagerView::dataChanged, this, &MainWindow::refreshAllViews);
    connect(m_recovery, &ScheduleRecoveryView::dataChanged, this, &MainWindow::refreshAllViews);
    connect(m_analytics, &AnalyticsView::dataChanged, this, &MainWindow::refreshAllViews);
    connect(m_focusTimer, &FocusTimerView::dataChanged, this, &MainWindow::refreshAllViews);
    connect(m_settings, &SettingsView::dataChanged, this, &MainWindow::refreshAllViews);

    m_stack->setCurrentIndex(0);
}

void MainWindow::onTabChanged(int index)
{
    if (index < 0 || index >= m_stack->count()) {
        return;
    }

    m_stack->setCurrentIndex(index);

    switch (index) {
    case 0: m_dashboard->refreshData(); break;
    case 1: m_taskManager->refreshTasks(); break;
    case 2: m_recovery->refreshData(); break;
    case 3: m_analytics->refreshData(); break;
    case 4: m_focusTimer->refreshData(); break;
    case 5: m_settings->refreshData(); break;
    default: break;
    }
}

void MainWindow::onNewTaskRequested()
{
    m_sidebar->setCurrentIndex(1);
    onTabChanged(1);
    m_taskManager->onNewTaskClicked();
    refreshAllViews();
}

void MainWindow::refreshAllViews()
{
    m_dashboard->refreshData();
    m_taskManager->refreshTasks();
    m_recovery->refreshData();
    m_analytics->refreshData();
    m_focusTimer->refreshData();
}
