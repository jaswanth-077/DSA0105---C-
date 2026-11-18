#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include "../../services/WorkloadBalancer.h"

class SubjectProgressWidget : public QWidget {
    Q_OBJECT

public:
    explicit SubjectProgressWidget(QWidget* parent = nullptr);

    void setSubjects(const QVector<SubjectProgress>& subjects);

private:
    QVBoxLayout* m_listLayout;
};
