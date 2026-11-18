#pragma once

#include <QWidget>
#include <QString>
#include <QColor>
#include <QVector>

struct PieSlice {
    QString label;
    int count;
    QColor color;
};

class StatusPieChartWidget : public QWidget {
    Q_OBJECT

public:
    explicit StatusPieChartWidget(QWidget* parent = nullptr);

    void setSlices(const QVector<PieSlice>& slices, const QString& centerLabel = "Tasks");

protected:
    void paintEvent(QPaintEvent* event) override;
    QSize sizeHint() const override;

private:
    QVector<PieSlice> m_slices;
    QString m_centerLabel;
    int m_totalCount;
};
