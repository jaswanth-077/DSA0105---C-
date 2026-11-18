#pragma once

#include <QString>
#include <QDate>
#include "Task.h"

class ScheduleEntry {
public:
    ScheduleEntry();
    ScheduleEntry(int id,
                  int taskId,
                  const QString& taskTitle,
                  const QString& subject,
                  TaskPriority priority,
                  const QDate& scheduledDate,
                  double allocatedHours,
                  bool isCompleted,
                  const QString& notes);

    int id() const { return m_id; }
    int taskId() const { return m_taskId; }
    QString taskTitle() const { return m_taskTitle; }
    QString subject() const { return m_subject; }
    TaskPriority priority() const { return m_priority; }
    QDate scheduledDate() const { return m_scheduledDate; }
    double allocatedHours() const { return m_allocatedHours; }
    bool isCompleted() const { return m_isCompleted; }
    QString notes() const { return m_notes; }

    void setId(int id) { m_id = id; }
    void setTaskId(int taskId) { m_taskId = taskId; }
    void setTaskTitle(const QString& title) { m_taskTitle = title; }
    void setSubject(const QString& subject) { m_subject = subject; }
    void setPriority(TaskPriority priority) { m_priority = priority; }
    void setScheduledDate(const QDate& date) { m_scheduledDate = date; }
    void setAllocatedHours(double hours) { m_allocatedHours = hours; }
    void setIsCompleted(bool completed) { m_isCompleted = completed; }
    void setNotes(const QString& notes) { m_notes = notes; }

private:
    int m_id;
    int m_taskId;
    QString m_taskTitle;
    QString m_subject;
    TaskPriority m_priority;
    QDate m_scheduledDate;
    double m_allocatedHours;
    bool m_isCompleted;
    QString m_notes;
};
