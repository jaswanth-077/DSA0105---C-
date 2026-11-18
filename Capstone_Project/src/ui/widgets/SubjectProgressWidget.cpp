#include "SubjectProgressWidget.h"
#include <QLabel>
#include <QProgressBar>
#include <QHBoxLayout>

SubjectProgressWidget::SubjectProgressWidget(QWidget* parent)
    : QWidget(parent)
{
    m_listLayout = new QVBoxLayout(this);
    m_listLayout->setContentsMargins(0, 0, 0, 0);
    m_listLayout->setSpacing(14);
}

void SubjectProgressWidget::setSubjects(const QVector<SubjectProgress>& subjects)
{
    // Clear previous items
    QLayoutItem* item;
    while ((item = m_listLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }

    if (subjects.isEmpty()) {
        auto* emptyLabel = new QLabel("No subjects registered yet.", this);
        emptyLabel->setStyleSheet("color: #94A3B8; font-size: 13px; font-style: italic;");
        m_listLayout->addWidget(emptyLabel);
        return;
    }

    for (const auto& sp : subjects) {
        auto* container = new QWidget(this);
        auto* rowLayout = new QVBoxLayout(container);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        rowLayout->setSpacing(4);

        auto* headerLayout = new QHBoxLayout();
        auto* nameLabel = new QLabel(sp.subject, container);
        nameLabel->setStyleSheet("font-weight: 600; color: #1E293B; font-size: 13px;");

        auto* statsLabel = new QLabel(
            QString("%1/%2 completed • %3h remaining")
                .arg(sp.completedTasks)
                .arg(sp.totalTasks)
                .arg(QString::number(sp.remainingHours, 'f', 1)),
            container
        );
        statsLabel->setStyleSheet("color: #64748B; font-size: 12px;");

        auto* pctLabel = new QLabel(QString("%1%").arg(QString::number(sp.completionRatePercent, 'f', 0)), container);
        pctLabel->setStyleSheet("font-weight: 700; color: #4F46E5; font-size: 13px;");

        headerLayout->addWidget(nameLabel);
        headerLayout->addWidget(statsLabel);
        headerLayout->addStretch();
        headerLayout->addWidget(pctLabel);

        auto* bar = new QProgressBar(container);
        bar->setRange(0, 100);
        bar->setValue(static_cast<int>(sp.completionRatePercent));
        bar->setTextVisible(false);
        bar->setFixedHeight(7);

        // Gradient or styling
        if (sp.completionRatePercent >= 100.0) {
            bar->setStyleSheet("QProgressBar::chunk { background-color: #10B981; border-radius: 3px; }");
        } else {
            bar->setStyleSheet("QProgressBar::chunk { background-color: #6366F1; border-radius: 3px; }");
        }

        rowLayout->addLayout(headerLayout);
        rowLayout->addWidget(bar);
        m_listLayout->addWidget(container);
    }
}
