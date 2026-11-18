#include "StatusPieChartWidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <algorithm>

StatusPieChartWidget::StatusPieChartWidget(QWidget* parent)
    : QWidget(parent)
    , m_centerLabel("Tasks")
    , m_totalCount(0)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    setMinimumHeight(220);
}

void StatusPieChartWidget::setSlices(const QVector<PieSlice>& slices, const QString& centerLabel)
{
    m_slices = slices;
    m_centerLabel = centerLabel;
    m_totalCount = 0;
    for (const auto& s : m_slices) {
        m_totalCount += s.count;
    }
    update();
}

QSize StatusPieChartWidget::sizeHint() const
{
    return QSize(320, 220);
}

void StatusPieChartWidget::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int w = width();
    int h = height();

    if (m_totalCount == 0) {
        painter.setPen(QColor("#94A3B8"));
        painter.setFont(QFont("Segoe UI", 12));
        painter.drawText(rect(), Qt::AlignCenter, "No task distribution data");
        return;
    }

    // Layout: Donut on left, Legend on right
    int donutDiameter = std::min(h - 30, (w / 2) - 20);
    donutDiameter = std::max(120, std::min(170, donutDiameter));
    int donutX = 20;
    int donutY = (h - donutDiameter) / 2;
    QRect donutRect(donutX, donutY, donutDiameter, donutDiameter);

    // Draw Donut Slices
    int startAngle = 90 * 16;
    for (const auto& slice : m_slices) {
        if (slice.count <= 0) continue;

        int spanAngle = static_cast<int>((static_cast<double>(slice.count) / m_totalCount) * 360.0 * 16.0);
        painter.setBrush(slice.color);
        painter.setPen(Qt::NoPen);
        painter.drawPie(donutRect, startAngle, spanAngle);
        startAngle += spanAngle;
    }

    // Inner Hole (Donut effect)
    int holeDiameter = static_cast<int>(donutDiameter * 0.65);
    int holeX = donutX + (donutDiameter - holeDiameter) / 2;
    int holeY = donutY + (donutDiameter - holeDiameter) / 2;
    QRect holeRect(holeX, holeY, holeDiameter, holeDiameter);

    painter.setBrush(QColor("#FFFFFF"));
    painter.drawEllipse(holeRect);

    // Center Text inside Donut
    painter.setPen(QColor("#0F172A"));
    painter.setFont(QFont("Segoe UI", 16, QFont::Bold));
    painter.drawText(holeRect.adjusted(0, -10, 0, 0), Qt::AlignCenter, QString::number(m_totalCount));

    painter.setPen(QColor("#64748B"));
    painter.setFont(QFont("Segoe UI", 9));
    painter.drawText(holeRect.adjusted(0, 24, 0, 0), Qt::AlignCenter, m_centerLabel);

    // Draw Legend on the right
    int legendX = donutX + donutDiameter + 25;
    int legendY = (h - (m_slices.size() * 28)) / 2;
    if (legendY < 15) legendY = 15;

    for (int i = 0; i < m_slices.size(); ++i) {
        const auto& slice = m_slices[i];
        int itemY = legendY + (i * 28);

        // Color dot
        painter.setBrush(slice.color);
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(QRect(legendX, itemY + 4, 10, 10), 3, 3);

        // Label
        painter.setPen(QColor("#334155"));
        painter.setFont(QFont("Segoe UI", 10, QFont::DemiBold));
        painter.drawText(QRect(legendX + 16, itemY, 110, 20), Qt::AlignLeft | Qt::AlignVCenter, slice.label);

        // Count & Percentage
        double pct = (m_totalCount > 0) ? ((static_cast<double>(slice.count) / m_totalCount) * 100.0) : 0.0;
        QString countStr = QString("%1 (%2%)").arg(slice.count).arg(pct, 0, 'f', 0);
        painter.setPen(QColor("#64748B"));
        painter.setFont(QFont("Segoe UI", 9));
        painter.drawText(QRect(legendX + 125, itemY, 70, 20), Qt::AlignLeft | Qt::AlignVCenter, countStr);
    }
}
