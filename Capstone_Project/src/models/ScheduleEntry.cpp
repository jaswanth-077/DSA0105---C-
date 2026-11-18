#include "ScheduleEntry.h"

ScheduleEntry::ScheduleEntry()
    : m_id(-1)
    , m_taskId(-1)
    , m_priority(TaskPriority::Medium)
    , m_scheduledDate(QDate::currentDate())
    , m_allocatedHours(1.0)
    , m_isCompleted(false)
{
}

ScheduleEntry::ScheduleEntry(int id,
                             int taskId,
                             const QString& taskTitle,
                             const QString& subject,
                             TaskPriority priority,
                             const QDate& scheduledDate,
                             double allocatedHours,
                             bool isCompleted,
                             const QString& notes)
    : m_id(id)
    , m_taskId(taskId)
    , m_taskTitle(taskTitle)
    , m_subject(subject)
    , m_priority(priority)
    , m_scheduledDate(scheduledDate)
    , m_allocatedHours(allocatedHours)
    , m_isCompleted(isCompleted)
    , m_notes(notes)
{
}
