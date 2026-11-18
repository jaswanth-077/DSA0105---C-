#pragma once

#include <QWidget>
#include <QDate>
#include <QVector>

struct ChartDayData {
    QDate date;
    QString dayName;
    double scheduledHours;
    double capacityHours;
    bool isOverloaded;
};

class WorkloadChartWidget : public QWidget {
    Q_OBJECT

public:
    explicit WorkloadChartWidget(QWidget* parent = nullptr);

    void setData(const QVector<ChartDayData>& data);
    void setHoveredIndex(int index);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    QSize sizeHint() const override;

private:
    QVector<ChartDayData> m_data;
    int m_hoveredIndex;
};
