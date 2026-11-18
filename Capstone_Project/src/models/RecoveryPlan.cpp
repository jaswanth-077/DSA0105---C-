#include "RecoveryPlan.h"

RecoveryPlan::RecoveryPlan()
    : m_conflictsResolved(0)
    , m_feasibilityScore(100.0)
    , m_summary("No schedule recovery adjustments needed at this time.")
{
}

void RecoveryPlan::addAction(const RescheduleAction& action)
{
    m_actions.append(action);
}
