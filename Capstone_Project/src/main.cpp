#include <QApplication>
#include <QFile>
#include <QMessageBox>

#include "database/DatabaseManager.h"
#include "ui/MainWindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QApplication::setOrganizationName("SmartStudy");
    QApplication::setApplicationName("SmartStudyPlanner");
    QApplication::setApplicationVersion("1.0.0");

    // Load the bundled theme
    QFile styleFile(":/styles.qss");
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        app.setStyleSheet(QString::fromUtf8(styleFile.readAll()));
    }

    if (!DatabaseManager::instance().initDatabase()) {
        QMessageBox::critical(
            nullptr,
            "Database Initialization Failed",
            "Smart Study Planner could not open its local SQLite database.\n"
            "Please check that the application data directory is writable and try again.");
        return 1;
    }

    MainWindow window;
    window.show();

    return app.exec();
}
