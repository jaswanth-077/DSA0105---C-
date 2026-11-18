#pragma once

#include <QString>

// Central definition of the SQLite schema used by Smart Study Planner.
// Kept in one place so the DatabaseManager and any future migration code
// agree on table names and column layouts.
namespace DatabaseSchema {

inline constexpr int kVersion = 1;

inline const QString kCreateTasksTable = R"(
    CREATE TABLE IF NOT EXISTS tasks (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        title TEXT NOT NULL,
        description TEXT,
        subject TEXT NOT NULL,
        task_type TEXT NOT NULL,
        priority TEXT NOT NULL,
        created_at TEXT NOT NULL,
        deadline TEXT NOT NULL,
        estimated_duration REAL NOT NULL,
        remaining_duration REAL NOT NULL,
        completion_percentage INTEGER NOT NULL DEFAULT 0,
        status TEXT NOT NULL
    );
)";

inline const QString kCreateScheduleEntriesTable = R"(
    CREATE TABLE IF NOT EXISTS schedule_entries (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        task_id INTEGER NOT NULL,
        scheduled_date TEXT NOT NULL,
        allocated_hours REAL NOT NULL,
        is_completed INTEGER DEFAULT 0,
        notes TEXT,
        FOREIGN KEY (task_id) REFERENCES tasks(id) ON DELETE CASCADE
    );
)";

inline const QString kCreateStudySessionsTable = R"(
    CREATE TABLE IF NOT EXISTS study_sessions (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        task_id INTEGER NOT NULL,
        session_date TEXT NOT NULL,
        duration_minutes INTEGER NOT NULL,
        notes TEXT,
        FOREIGN KEY (task_id) REFERENCES tasks(id) ON DELETE CASCADE
    );
)";

inline const QString kCreateUserPreferencesTable = R"(
    CREATE TABLE IF NOT EXISTS user_preferences (
        pref_key TEXT PRIMARY KEY,
        pref_value TEXT NOT NULL
    );
)";

} // namespace DatabaseSchema
