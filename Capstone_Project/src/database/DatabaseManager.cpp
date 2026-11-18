#include "DatabaseManager.h"
#include "DatabaseSchema.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QVariant>

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager()
{
}

DatabaseManager::~DatabaseManager()
{
    closeDatabase();
}

bool DatabaseManager::initDatabase(const QString& path)
{
    if (m_db.isOpen()) {
        return true;
    }

    if (path.isEmpty()) {
        QString appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        if (appDataDir.isEmpty()) {
            appDataDir = "./data";
        }
        QDir dir(appDataDir);
        if (!dir.exists()) {
            dir.mkpath(".");
        }
        m_dbPath = dir.filePath("smart_study_planner.db");
    } else {
        m_dbPath = path;
        QFileInfo fi(m_dbPath);
        QDir dir = fi.dir();
        if (!dir.exists()) {
            dir.mkpath(".");
        }
    }

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(m_dbPath);

    if (!m_db.open()) {
        qWarning() << "Failed to open SQLite database at" << m_dbPath << ":" << m_db.lastError().text();
        return false;
    }

    // Enable foreign keys
    QSqlQuery query(m_db);
    query.exec("PRAGMA foreign_keys = ON;");

    if (!createTables()) {
        return false;
    }

    // If completely empty, populate initial realistic starter data
    QSqlQuery countQuery("SELECT COUNT(*) FROM tasks", m_db);
    if (countQuery.next() && countQuery.value(0).toInt() == 0) {
        seedRealisticData();
    }

    return true;
}

void DatabaseManager::closeDatabase()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool DatabaseManager::isOpen() const
{
    return m_db.isOpen();
}

bool DatabaseManager::createTables()
{
    QSqlQuery query(m_db);

    // 1. Tasks Table
    if (!query.exec(DatabaseSchema::kCreateTasksTable)) {
        qWarning() << "Error creating tasks table:" << query.lastError().text();
        return false;
    }

    // 2. Schedule Entries Table
    if (!query.exec(DatabaseSchema::kCreateScheduleEntriesTable)) {
        qWarning() << "Error creating schedule_entries table:" << query.lastError().text();
        return false;
    }

    // 3. Study Sessions Table
    if (!query.exec(DatabaseSchema::kCreateStudySessionsTable)) {
        qWarning() << "Error creating study_sessions table:" << query.lastError().text();
        return false;
    }

    // 4. User Preferences Table
    if (!query.exec(DatabaseSchema::kCreateUserPreferencesTable)) {
        qWarning() << "Error creating user_preferences table:" << query.lastError().text();
        return false;
    }

    return true;
}

int DatabaseManager::addTask(const Task& task)
{
    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT INTO tasks (title, description, subject, task_type, priority,
                           created_at, deadline, estimated_duration, remaining_duration,
                           completion_percentage, status)
        VALUES (:title, :description, :subject, :task_type, :priority,
                :created_at, :deadline, :estimated_duration, :remaining_duration,
                :completion_percentage, :status)
    )");

    query.bindValue(":title", task.title());
    query.bindValue(":description", task.description());
    query.bindValue(":subject", task.subject());
    query.bindValue(":task_type", Task::typeToString(task.type()));
    query.bindValue(":priority", Task::priorityToString(task.priority()));
    query.bindValue(":created_at", task.createdAt().toString(Qt::ISODate));
    query.bindValue(":deadline", task.deadline().toString(Qt::ISODate));
    query.bindValue(":estimated_duration", task.estimatedDuration());
    query.bindValue(":remaining_duration", task.remainingDuration());
    query.bindValue(":completion_percentage", task.completionPercentage());
    query.bindValue(":status", Task::statusToString(task.status()));

    if (!query.exec()) {
        qWarning() << "Failed to add task:" << query.lastError().text();
        return -1;
    }

    return query.lastInsertId().toInt();
}

bool DatabaseManager::updateTask(const Task& task)
{
    QSqlQuery query(m_db);
    query.prepare(R"(
        UPDATE tasks SET
            title = :title,
            description = :description,
            subject = :subject,
            task_type = :task_type,
            priority = :priority,
            created_at = :created_at,
            deadline = :deadline,
            estimated_duration = :estimated_duration,
            remaining_duration = :remaining_duration,
            completion_percentage = :completion_percentage,
            status = :status
        WHERE id = :id
    )");

    query.bindValue(":title", task.title());
    query.bindValue(":description", task.description());
    query.bindValue(":subject", task.subject());
    query.bindValue(":task_type", Task::typeToString(task.type()));
    query.bindValue(":priority", Task::priorityToString(task.priority()));
    query.bindValue(":created_at", task.createdAt().toString(Qt::ISODate));
    query.bindValue(":deadline", task.deadline().toString(Qt::ISODate));
    query.bindValue(":estimated_duration", task.estimatedDuration());
    query.bindValue(":remaining_duration", task.remainingDuration());
    query.bindValue(":completion_percentage", task.completionPercentage());
    query.bindValue(":status", Task::statusToString(task.status()));
    query.bindValue(":id", task.id());

    return query.exec();
}

bool DatabaseManager::deleteTask(int taskId)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM tasks WHERE id = :id");
    query.bindValue(":id", taskId);
    return query.exec();
}

static Task taskFromRecord(const QSqlQuery& query)
{
    int id = query.value("id").toInt();
    QString title = query.value("title").toString();
    QString description = query.value("description").toString();
    QString subject = query.value("subject").toString();
    TaskType type = Task::stringToType(query.value("task_type").toString());
    TaskPriority priority = Task::stringToPriority(query.value("priority").toString());
    QDateTime createdAt = QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
    QDateTime deadline = QDateTime::fromString(query.value("deadline").toString(), Qt::ISODate);
    double estimatedDuration = query.value("estimated_duration").toDouble();
    double remainingDuration = query.value("remaining_duration").toDouble();
    int completionPercentage = query.value("completion_percentage").toInt();
    TaskStatus status = Task::stringToStatus(query.value("status").toString());

    return Task(id, title, description, subject, type, priority, createdAt, deadline,
                estimatedDuration, remainingDuration, completionPercentage, status);
}

std::optional<Task> DatabaseManager::getTask(int taskId)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM tasks WHERE id = :id");
    query.bindValue(":id", taskId);

    if (query.exec() && query.next()) {
        return taskFromRecord(query);
    }
    return std::nullopt;
}

QVector<Task> DatabaseManager::getAllTasks()
{
    QVector<Task> tasks;
    QSqlQuery query("SELECT * FROM tasks ORDER BY deadline ASC", m_db);
    while (query.next()) {
        tasks.append(taskFromRecord(query));
    }
    return tasks;
}

QVector<Task> DatabaseManager::getActiveTasks()
{
    QVector<Task> tasks;
    QSqlQuery query("SELECT * FROM tasks WHERE status != 'Completed' ORDER BY deadline ASC", m_db);
    while (query.next()) {
        tasks.append(taskFromRecord(query));
    }
    return tasks;
}

QVector<Task> DatabaseManager::getTasksFiltered(const QString& search,
                                               const QString& subject,
                                               const QString& priority,
                                               const QString& status)
{
    QString sql = "SELECT * FROM tasks WHERE 1=1";
    if (!search.trimmed().isEmpty()) {
        sql += " AND (title LIKE :search OR description LIKE :search OR subject LIKE :search)";
    }
    if (!subject.isEmpty() && subject != "All Subjects") {
        sql += " AND subject = :subject";
    }
    if (!priority.isEmpty() && priority != "All Priorities") {
        sql += " AND priority = :priority";
    }
    if (!status.isEmpty() && status != "All Statuses") {
        sql += " AND status = :status";
    }
    sql += " ORDER BY deadline ASC";

    QSqlQuery query(m_db);
    query.prepare(sql);

    if (!search.trimmed().isEmpty()) {
        query.bindValue(":search", "%" + search.trimmed() + "%");
    }
    if (!subject.isEmpty() && subject != "All Subjects") {
        query.bindValue(":subject", subject);
    }
    if (!priority.isEmpty() && priority != "All Priorities") {
        query.bindValue(":priority", priority);
    }
    if (!status.isEmpty() && status != "All Statuses") {
        query.bindValue(":status", status);
    }

    QVector<Task> results;
    if (query.exec()) {
        while (query.next()) {
            results.append(taskFromRecord(query));
        }
    }
    return results;
}

QVector<QString> DatabaseManager::getDistinctSubjects()
{
    QVector<QString> subjects;
    QSqlQuery query("SELECT DISTINCT subject FROM tasks WHERE subject IS NOT NULL AND subject != '' ORDER BY subject ASC", m_db);
    while (query.next()) {
        subjects.append(query.value(0).toString());
    }
    return subjects;
}

int DatabaseManager::addScheduleEntry(const ScheduleEntry& entry)
{
    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT INTO schedule_entries (task_id, scheduled_date, allocated_hours, is_completed, notes)
        VALUES (:task_id, :scheduled_date, :allocated_hours, :is_completed, :notes)
    )");
    query.bindValue(":task_id", entry.taskId());
    query.bindValue(":scheduled_date", entry.scheduledDate().toString(Qt::ISODate));
    query.bindValue(":allocated_hours", entry.allocatedHours());
    query.bindValue(":is_completed", entry.isCompleted() ? 1 : 0);
    query.bindValue(":notes", entry.notes());

    if (!query.exec()) {
        qWarning() << "Failed to add schedule entry:" << query.lastError().text();
        return -1;
    }
    return query.lastInsertId().toInt();
}

bool DatabaseManager::updateScheduleEntry(const ScheduleEntry& entry)
{
    QSqlQuery query(m_db);
    query.prepare(R"(
        UPDATE schedule_entries SET
            task_id = :task_id,
            scheduled_date = :scheduled_date,
            allocated_hours = :allocated_hours,
            is_completed = :is_completed,
            notes = :notes
        WHERE id = :id
    )");
    query.bindValue(":task_id", entry.taskId());
    query.bindValue(":scheduled_date", entry.scheduledDate().toString(Qt::ISODate));
    query.bindValue(":allocated_hours", entry.allocatedHours());
    query.bindValue(":is_completed", entry.isCompleted() ? 1 : 0);
    query.bindValue(":notes", entry.notes());
    query.bindValue(":id", entry.id());

    return query.exec();
}

bool DatabaseManager::deleteScheduleEntry(int entryId)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM schedule_entries WHERE id = :id");
    query.bindValue(":id", entryId);
    return query.exec();
}

bool DatabaseManager::deleteScheduleEntriesForTask(int taskId)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM schedule_entries WHERE task_id = :task_id");
    query.bindValue(":task_id", taskId);
    return query.exec();
}

static ScheduleEntry entryFromJoinedQuery(const QSqlQuery& query)
{
    int id = query.value("id").toInt();
    int taskId = query.value("task_id").toInt();
    QString title = query.value("task_title").toString();
    QString subject = query.value("subject").toString();
    TaskPriority priority = Task::stringToPriority(query.value("priority").toString());
    QDate scheduledDate = QDate::fromString(query.value("scheduled_date").toString(), Qt::ISODate);
    double allocatedHours = query.value("allocated_hours").toDouble();
    bool isCompleted = query.value("is_completed").toInt() == 1;
    QString notes = query.value("notes").toString();

    return ScheduleEntry(id, taskId, title, subject, priority, scheduledDate, allocatedHours, isCompleted, notes);
}

QVector<ScheduleEntry> DatabaseManager::getAllScheduleEntries()
{
    QVector<ScheduleEntry> entries;
    const QString sql = R"(
        SELECT s.id, s.task_id, s.scheduled_date, s.allocated_hours, s.is_completed, s.notes,
               t.title AS task_title, t.subject, t.priority
        FROM schedule_entries s
        JOIN tasks t ON s.task_id = t.id
        ORDER BY s.scheduled_date ASC
    )";
    QSqlQuery query(sql, m_db);
    while (query.next()) {
        entries.append(entryFromJoinedQuery(query));
    }
    return entries;
}

QVector<ScheduleEntry> DatabaseManager::getScheduleEntriesForDate(const QDate& date)
{
    QVector<ScheduleEntry> entries;
    const QString sql = R"(
        SELECT s.id, s.task_id, s.scheduled_date, s.allocated_hours, s.is_completed, s.notes,
               t.title AS task_title, t.subject, t.priority
        FROM schedule_entries s
        JOIN tasks t ON s.task_id = t.id
        WHERE s.scheduled_date = :scheduled_date
        ORDER BY t.priority DESC, s.allocated_hours DESC
    )";
    QSqlQuery query(m_db);
    query.prepare(sql);
    query.bindValue(":scheduled_date", date.toString(Qt::ISODate));
    if (query.exec()) {
        while (query.next()) {
            entries.append(entryFromJoinedQuery(query));
        }
    }
    return entries;
}

QVector<ScheduleEntry> DatabaseManager::getScheduleEntriesBetween(const QDate& start, const QDate& end)
{
    QVector<ScheduleEntry> entries;
    const QString sql = R"(
        SELECT s.id, s.task_id, s.scheduled_date, s.allocated_hours, s.is_completed, s.notes,
               t.title AS task_title, t.subject, t.priority
        FROM schedule_entries s
        JOIN tasks t ON s.task_id = t.id
        WHERE s.scheduled_date >= :start_date AND s.scheduled_date <= :end_date
        ORDER BY s.scheduled_date ASC, t.priority DESC
    )";
    QSqlQuery query(m_db);
    query.prepare(sql);
    query.bindValue(":start_date", start.toString(Qt::ISODate));
    query.bindValue(":end_date", end.toString(Qt::ISODate));
    if (query.exec()) {
        while (query.next()) {
            entries.append(entryFromJoinedQuery(query));
        }
    }
    return entries;
}

double DatabaseManager::getTotalScheduledHoursForDate(const QDate& date)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT SUM(allocated_hours) FROM schedule_entries WHERE scheduled_date = :date");
    query.bindValue(":date", date.toString(Qt::ISODate));
    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }
    return 0.0;
}

int DatabaseManager::addStudySession(const StudySession& session)
{
    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT INTO study_sessions (task_id, session_date, duration_minutes, notes)
        VALUES (:task_id, :session_date, :duration_minutes, :notes)
    )");
    query.bindValue(":task_id", session.taskId());
    query.bindValue(":session_date", session.sessionDate().toString(Qt::ISODate));
    query.bindValue(":duration_minutes", session.durationMinutes());
    query.bindValue(":notes", session.notes());

    if (!query.exec()) {
        qWarning() << "Failed to add study session:" << query.lastError().text();
        return -1;
    }
    return query.lastInsertId().toInt();
}

bool DatabaseManager::deleteStudySession(int sessionId)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM study_sessions WHERE id = :id");
    query.bindValue(":id", sessionId);
    return query.exec();
}

QVector<StudySession> DatabaseManager::getAllStudySessions()
{
    QVector<StudySession> sessions;
    const QString sql = R"(
        SELECT s.id, s.task_id, s.session_date, s.duration_minutes, s.notes,
               t.title AS task_title
        FROM study_sessions s
        JOIN tasks t ON s.task_id = t.id
        ORDER BY s.session_date DESC
    )";
    QSqlQuery query(sql, m_db);
    while (query.next()) {
        int id = query.value("id").toInt();
        int taskId = query.value("task_id").toInt();
        QString title = query.value("task_title").toString();
        QDateTime dt = QDateTime::fromString(query.value("session_date").toString(), Qt::ISODate);
        int mins = query.value("duration_minutes").toInt();
        QString notes = query.value("notes").toString();
        sessions.append(StudySession(id, taskId, title, dt, mins, notes));
    }
    return sessions;
}

QVector<StudySession> DatabaseManager::getStudySessionsForTask(int taskId)
{
    QVector<StudySession> sessions;
    const QString sql = R"(
        SELECT s.id, s.task_id, s.session_date, s.duration_minutes, s.notes,
               t.title AS task_title
        FROM study_sessions s
        JOIN tasks t ON s.task_id = t.id
        WHERE s.task_id = :task_id
        ORDER BY s.session_date DESC
    )";
    QSqlQuery query(m_db);
    query.prepare(sql);
    query.bindValue(":task_id", taskId);
    if (query.exec()) {
        while (query.next()) {
            int id = query.value("id").toInt();
            QString title = query.value("task_title").toString();
            QDateTime dt = QDateTime::fromString(query.value("session_date").toString(), Qt::ISODate);
            int mins = query.value("duration_minutes").toInt();
            QString notes = query.value("notes").toString();
            sessions.append(StudySession(id, taskId, title, dt, mins, notes));
        }
    }
    return sessions;
}

double DatabaseManager::getTotalActualHoursForTask(int taskId)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT SUM(duration_minutes) FROM study_sessions WHERE task_id = :task_id");
    query.bindValue(":task_id", taskId);
    if (query.exec() && query.next()) {
        return query.value(0).toDouble() / 60.0;
    }
    return 0.0;
}

double DatabaseManager::getTotalActualHoursBetween(const QDateTime& start, const QDateTime& end)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT SUM(duration_minutes) FROM study_sessions WHERE session_date >= :start AND session_date <= :end");
    query.bindValue(":start", start.toString(Qt::ISODate));
    query.bindValue(":end", end.toString(Qt::ISODate));
    if (query.exec() && query.next()) {
        return query.value(0).toDouble() / 60.0;
    }
    return 0.0;
}

QString DatabaseManager::getPreference(const QString& key, const QString& defaultValue)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT pref_value FROM user_preferences WHERE pref_key = :key");
    query.bindValue(":key", key);
    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }
    return defaultValue;
}

bool DatabaseManager::setPreference(const QString& key, const QString& value)
{
    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT INTO user_preferences (pref_key, pref_value)
        VALUES (:key, :value)
        ON CONFLICT(pref_key) DO UPDATE SET pref_value = :value
    )");
    query.bindValue(":key", key);
    query.bindValue(":value", value);
    return query.exec();
}

void DatabaseManager::clearAllData()
{
    QSqlQuery query(m_db);
    query.exec("DELETE FROM study_sessions;");
    query.exec("DELETE FROM schedule_entries;");
    query.exec("DELETE FROM tasks;");
}

void DatabaseManager::seedRealisticData()
{
    clearAllData();

    QDateTime now = QDateTime::currentDateTime();
    QDate today = now.date();

    // Default Preferences
    setPreference("weekday_capacity_hours", "5.0");
    setPreference("weekend_capacity_hours", "6.5");
    setPreference("theme", "Light");
    setPreference("auto_balance", "true");

    // Task 1: Imminent & high priority (Critical deadline in 24 hours)
    Task t1(
        -1,
        "Operating Systems Virtual Memory Lab",
        "Implement page table lookups, TLB caching simulation, and LRU replacement algorithm.",
        "Computer Science",
        TaskType::Assignment,
        TaskPriority::Critical,
        now.addDays(-4),
        now.addDays(1).addSecs(3600 * 3), // Tomorrow
        6.0,
        3.5,
        40,
        TaskStatus::InProgress
    );
    int id1 = addTask(t1);

    // Task 2: High priority Exam Preparation (Midterm in 4 days)
    Task t2(
        -1,
        "Discrete Mathematics Midterm Prep",
        "Master graph theory proofs, planar graphs, Eulerian circuits, and recurrence relations.",
        "Mathematics",
        TaskType::ExamPrep,
        TaskPriority::High,
        now.addDays(-6),
        now.addDays(4).addSecs(3600 * 2),
        10.0,
        6.0,
        40,
        TaskStatus::InProgress
    );
    int id2 = addTask(t2);

    // Task 3: Group Project Milestone (in 7 days)
    Task t3(
        -1,
        "Distributed Database Prototype",
        "Build Raft consensus leader election module and cluster state replication tests.",
        "Software Engineering",
        TaskType::Project,
        TaskPriority::High,
        now.addDays(-8),
        now.addDays(7),
        14.0,
        9.0,
        35,
        TaskStatus::InProgress
    );
    int id3 = addTask(t3);

    // Task 4: Physics Problem Set (in 2 days)
    Task t4(
        -1,
        "Electromagnetism Problem Set 5",
        "Solve Maxwell equation boundary condition problems and Poynting vector flux exercises.",
        "Physics",
        TaskType::Assignment,
        TaskPriority::Medium,
        now.addDays(-3),
        now.addDays(2),
        4.0,
        1.5,
        60,
        TaskStatus::InProgress
    );
    int id4 = addTask(t4);

    // Task 5: Weekly Literature Review (Revision)
    Task t5(
        -1,
        "Computer Architecture Cache Hierarchy Review",
        "Read Hennessy & Patterson Chapter 2 on memory hierarchy performance and non-blocking caches.",
        "Computer Science",
        TaskType::Revision,
        TaskPriority::Low,
        now.addDays(-5),
        now.addDays(6),
        3.0,
        3.0,
        0,
        TaskStatus::NotStarted
    );
    int id5 = addTask(t5);

    // Task 6: Completed Task
    Task t6(
        -1,
        "Linear Algebra Homework 3",
        "Eigenvalues, eigenvectors, and singular value decomposition exercises completed and submitted.",
        "Mathematics",
        TaskType::Assignment,
        TaskPriority::Medium,
        now.addDays(-7),
        now.addDays(-1),
        4.5,
        0.0,
        100,
        TaskStatus::Completed
    );
    int id6 = addTask(t6);

    // Task 7: Overdue / Delayed Task
    Task t7(
        -1,
        "Technical Communication Research Proposal",
        "Literature survey draft and problem formulation for scientific grant proposal assignment.",
        "Humanities",
        TaskType::Assignment,
        TaskPriority::Medium,
        now.addDays(-10),
        now.addDays(-1).addSecs(-3600 * 8), // Yesterday overdue
        5.0,
        2.5,
        50,
        TaskStatus::Delayed
    );
    int id7 = addTask(t7);

    // Seed Schedule Entries
    // Today
    addScheduleEntry(ScheduleEntry(-1, id1, t1.title(), t1.subject(), t1.priority(), today, 2.5, false, "Finish TLB simulation"));
    addScheduleEntry(ScheduleEntry(-1, id4, t4.title(), t4.subject(), t4.priority(), today, 1.5, false, "Problem set questions 1 to 4"));

    // Tomorrow (Overloaded day initially to demonstrate smart balancing!)
    addScheduleEntry(ScheduleEntry(-1, id1, t1.title(), t1.subject(), t1.priority(), today.addDays(1), 3.0, false, "Final debugging and report"));
    addScheduleEntry(ScheduleEntry(-1, id2, t2.title(), t2.subject(), t2.priority(), today.addDays(1), 2.5, false, "Graph theory theorems"));
    addScheduleEntry(ScheduleEntry(-1, id7, t7.title(), t7.subject(), t7.priority(), today.addDays(1), 1.5, false, "Complete overdue draft"));

    // Day after tomorrow
    addScheduleEntry(ScheduleEntry(-1, id2, t2.title(), t2.subject(), t2.priority(), today.addDays(2), 2.0, false, "Recurrence relations"));
    addScheduleEntry(ScheduleEntry(-1, id3, t3.title(), t3.subject(), t3.priority(), today.addDays(2), 2.5, false, "Raft RPC handlers"));

    // Day 3
    addScheduleEntry(ScheduleEntry(-1, id2, t2.title(), t2.subject(), t2.priority(), today.addDays(3), 2.0, false, "Mock exam timed test"));
    addScheduleEntry(ScheduleEntry(-1, id3, t3.title(), t3.subject(), t3.priority(), today.addDays(3), 2.0, false, "State machine replication"));

    // Seed Study Sessions (Actual study hours logged)
    addStudySession(StudySession(-1, id1, t1.title(), now.addDays(-2), 120, "Set up repository and paging structs"));
    addStudySession(StudySession(-1, id1, t1.title(), now.addDays(-1), 60, "Debugging segmentation fault in page walker"));
    addStudySession(StudySession(-1, id2, t2.title(), now.addDays(-3), 150, "Eulerian graph traversal review"));
    addStudySession(StudySession(-1, id2, t2.title(), now.addDays(-1), 90, "Worked on induction proofs"));
    addStudySession(StudySession(-1, id4, t4.title(), now.addDays(-2), 90, "Solved questions 1 and 2"));
    addStudySession(StudySession(-1, id6, t6.title(), now.addDays(-4), 180, "Finished SVD problems"));
    addStudySession(StudySession(-1, id6, t6.title(), now.addDays(-2), 90, "Final check and PDF export"));
}
