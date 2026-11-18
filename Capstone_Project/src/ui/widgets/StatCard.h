#pragma once

#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

class StatCard : public QFrame {
    Q_OBJECT

public:
    explicit StatCard(const QString& icon,
                      const QString& label,
                      const QString& value,
                      const QString& subtext = "",
                      const QString& badgeColor = "#4F46E5",
                      QWidget* parent = nullptr);

    void setValue(const QString& value);
    void setSubtext(const QString& subtext);

private:
    QLabel* m_iconLabel;
    QLabel* m_labelLabel;
    QLabel* m_valueLabel;
    QLabel* m_subtextLabel;
};
