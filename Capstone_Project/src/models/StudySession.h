#pragma once

#include <QString>
#include <QDateTime>

class StudySession {
public:
    StudySession();
    StudySession(int id,
                 int taskId,
                 const QString& taskTitle,
                 const QDateTime& sessionDate,
                 int durationMinutes,
                 const QString& notes);

    int id() const { return m_id; }
    int taskId() const { return m_taskId; }
    QString taskTitle() const { return m_taskTitle; }
    QDateTime sessionDate() const { return m_sessionDate; }
    int durationMinutes() const { return m_durationMinutes; }
    double durationHours() const { return static_cast<double>(m_durationMinutes) / 60.0; }
    QString notes() const { return m_notes; }

    void setId(int id) { m_id = id; }
    void setTaskId(int taskId) { m_taskId = taskId; }
    void setTaskTitle(const QString& title) { m_taskTitle = title; }
    void setSessionDate(const QDateTime& dt) { m_sessionDate = dt; }
    void setDurationMinutes(int mins) { m_durationMinutes = mins; }
    void setNotes(const QString& notes) { m_notes = notes; }

private:
    int m_id;
    int m_taskId;
    QString m_taskTitle;
    QDateTime m_sessionDate;
    int m_durationMinutes;
    QString m_notes;
};
