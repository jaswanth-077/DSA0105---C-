#pragma once

#include <QWidget>
#include <QPushButton>
#include <QButtonGroup>
#include <QVBoxLayout>

class Sidebar : public QWidget {
    Q_OBJECT

public:
    explicit Sidebar(QWidget* parent = nullptr);

    void setCurrentIndex(int index);

signals:
    void tabChanged(int index);
    void newTaskRequested();

private:
    QButtonGroup* m_buttonGroup;
    QVBoxLayout* m_navLayout;
    QVector<QPushButton*> m_navButtons;
};
