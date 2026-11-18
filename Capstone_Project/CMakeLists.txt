cmake_minimum_required(VERSION 3.16)
project(SmartStudyPlanner VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql Svg SvgWidgets)

set(SOURCES
    src/main.cpp
    src/models/Task.cpp
    src/models/ScheduleEntry.cpp
    src/models/StudySession.cpp
    src/models/DeadlineRisk.cpp
    src/models/RecoveryPlan.cpp
    src/database/DatabaseManager.cpp
    src/services/TaskService.cpp
    src/services/DeadlineAnalyzer.cpp
    src/services/RecoveryScheduler.cpp
    src/services/WorkloadBalancer.cpp
    src/ui/MainWindow.cpp
    src/ui/Sidebar.cpp
    src/ui/DashboardView.cpp
    src/ui/TaskManagerView.cpp
    src/ui/ScheduleRecoveryView.cpp
    src/ui/AnalyticsView.cpp
    src/ui/FocusTimerView.cpp
    src/ui/SettingsView.cpp
    src/ui/dialogs/TaskEditDialog.cpp
    src/ui/dialogs/LogSessionDialog.cpp
    src/ui/dialogs/RecoveryPlanDialog.cpp
    src/ui/widgets/StatCard.cpp
    src/ui/widgets/WorkloadChartWidget.cpp
    src/ui/widgets/StatusPieChartWidget.cpp
    src/ui/widgets/SubjectProgressWidget.cpp
)

set(HEADERS
    src/models/Task.h
    src/models/ScheduleEntry.h
    src/models/StudySession.h
    src/models/DeadlineRisk.h
    src/models/RecoveryPlan.h
    src/database/DatabaseManager.h
    src/services/TaskService.h
    src/services/DeadlineAnalyzer.h
    src/services/RecoveryScheduler.h
    src/services/WorkloadBalancer.h
    src/ui/MainWindow.h
    src/ui/Sidebar.h
    src/ui/DashboardView.h
    src/ui/TaskManagerView.h
    src/ui/ScheduleRecoveryView.h
    src/ui/AnalyticsView.h
    src/ui/FocusTimerView.h
    src/ui/SettingsView.h
    src/ui/dialogs/TaskEditDialog.h
    src/ui/dialogs/LogSessionDialog.h
    src/ui/dialogs/RecoveryPlanDialog.h
    src/ui/widgets/StatCard.h
    src/ui/widgets/WorkloadChartWidget.h
    src/ui/widgets/StatusPieChartWidget.h
    src/ui/widgets/SubjectProgressWidget.h
)

set(RESOURCES
    resources/resources.qrc
)

add_executable(SmartStudyPlanner WIN32
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

target_include_directories(SmartStudyPlanner PRIVATE
    src
    src/models
    src/database
    src/services
    src/ui
    src/ui/dialogs
    src/ui/widgets
)

target_link_libraries(SmartStudyPlanner PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
    Qt6::Svg
    Qt6::SvgWidgets
)
