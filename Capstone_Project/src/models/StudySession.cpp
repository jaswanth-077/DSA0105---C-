#include "StudySession.h"

StudySession::StudySession()
    : m_id(-1)
    , m_taskId(-1)
    , m_sessionDate(QDateTime::currentDateTime())
    , m_durationMinutes(30)
{
}

StudySession::StudySession(int id,
                           int taskId,
                           const QString& taskTitle,
                           const QDateTime& sessionDate,
                           int durationMinutes,
                           const QString& notes)
    : m_id(id)
    , m_taskId(taskId)
    , m_taskTitle(taskTitle)
    , m_sessionDate(sessionDate)
    , m_durationMinutes(durationMinutes)
    , m_notes(notes)
{
}
