#pragma once

#include <QString>
#include <QVector>
#include <QDate>
#include <optional>
#include <QSqlDatabase>
#include "../models/Task.h"
#include "../models/ScheduleEntry.h"
#include "../models/StudySession.h"

class DatabaseManager {
public:
    static DatabaseManager& instance();

    bool initDatabase(const QString& path = "");
    void closeDatabase();
    bool isOpen() const;

    // Task Operations
    int addTask(const Task& task);
    bool updateTask(const Task& task);
    bool deleteTask(int taskId);
    std::optional<Task> getTask(int taskId);
    QVector<Task> getAllTasks();
    QVector<Task> getActiveTasks();
    QVector<Task> getTasksFiltered(const QString& search,
                                   const QString& subject,
                                   const QString& priority,
                                   const QString& status);
    QVector<QString> getDistinctSubjects();

    // Schedule Operations
    int addScheduleEntry(const ScheduleEntry& entry);
    bool updateScheduleEntry(const ScheduleEntry& entry);
    bool deleteScheduleEntry(int entryId);
    bool deleteScheduleEntriesForTask(int taskId);
    QVector<ScheduleEntry> getAllScheduleEntries();
    QVector<ScheduleEntry> getScheduleEntriesForDate(const QDate& date);
    QVector<ScheduleEntry> getScheduleEntriesBetween(const QDate& start, const QDate& end);
    double getTotalScheduledHoursForDate(const QDate& date);

    // Study Session Operations
    int addStudySession(const StudySession& session);
    bool deleteStudySession(int sessionId);
    QVector<StudySession> getAllStudySessions();
    QVector<StudySession> getStudySessionsForTask(int taskId);
    double getTotalActualHoursForTask(int taskId);
    double getTotalActualHoursBetween(const QDateTime& start, const QDateTime& end);

    // User Preferences Operations
    QString getPreference(const QString& key, const QString& defaultValue = "");
    bool setPreference(const QString& key, const QString& value);

    // Database Reset and Sample Data
    void clearAllData();
    void seedRealisticData();

private:
    DatabaseManager();
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    bool createTables();

    QSqlDatabase m_db;
    QString m_dbPath;
};
