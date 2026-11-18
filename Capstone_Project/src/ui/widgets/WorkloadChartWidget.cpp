#include "WorkloadChartWidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <algorithm>
#include <cmath>

WorkloadChartWidget::WorkloadChartWidget(QWidget* parent)
    : QWidget(parent)
    , m_hoveredIndex(-1)
{
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    setMinimumHeight(220);
}

void WorkloadChartWidget::setData(const QVector<ChartDayData>& data)
{
    m_data = data;
    update();
}

void WorkloadChartWidget::setHoveredIndex(int index)
{
    if (m_hoveredIndex != index) {
        m_hoveredIndex = index;
        update();
    }
}

QSize WorkloadChartWidget::sizeHint() const
{
    return QSize(500, 240);
}

void WorkloadChartWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (m_data.isEmpty()) {
        QWidget::mouseMoveEvent(event);
        return;
    }

    int leftMargin = 45;
    int rightMargin = 20;
    int chartWidth = width() - leftMargin - rightMargin;
    int count = m_data.size();
    double colWidth = static_cast<double>(chartWidth) / count;

    int idx = static_cast<int>((event->pos().x() - leftMargin) / colWidth);
    if (idx >= 0 && idx < count) {
        setHoveredIndex(idx);
    } else {
        setHoveredIndex(-1);
    }

    QWidget::mouseMoveEvent(event);
}

void WorkloadChartWidget::leaveEvent(QEvent* event)
{
    setHoveredIndex(-1);
    QWidget::leaveEvent(event);
}

void WorkloadChartWidget::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int w = width();
    int h = height();

    // Background
    painter.fillRect(rect(), Qt::transparent);

    if (m_data.isEmpty()) {
        painter.setPen(QColor("#94A3B8"));
        painter.setFont(QFont("Segoe UI", 12));
        painter.drawText(rect(), Qt::AlignCenter, "No schedule data to display");
        return;
    }

    int leftMargin = 45;
    int rightMargin = 25;
    int topMargin = 30;
    int bottomMargin = 45;

    int chartWidth = w - leftMargin - rightMargin;
    int chartHeight = h - topMargin - bottomMargin;

    // Calculate max hours for scale
    double maxHours = 6.0;
    for (const auto& d : m_data) {
        maxHours = std::max(maxHours, std::max(d.scheduledHours, d.capacityHours));
    }
    maxHours = std::ceil(maxHours * 1.15); // Add 15% headroom

    // Draw horizontal grid lines
    painter.setFont(QFont("Segoe UI", 9));
    int gridLines = 4;
    for (int i = 0; i <= gridLines; ++i) {
        double val = (maxHours / gridLines) * i;
        int y = topMargin + chartHeight - static_cast<int>((val / maxHours) * chartHeight);

        // Grid line
        painter.setPen(QPen(QColor("#F1F5F9"), 1, Qt::SolidLine));
        painter.drawLine(leftMargin, y, w - rightMargin, y);

        // Y-axis label
        painter.setPen(QColor("#94A3B8"));
        painter.drawText(QRect(0, y - 8, leftMargin - 8, 16), Qt::AlignRight | Qt::AlignVCenter, QString::number(val, 'f', 0) + "h");
    }

    // Draw Capacity Threshold Line (from average capacity)
    double avgCap = 5.0;
    if (!m_data.isEmpty()) {
        avgCap = m_data.first().capacityHours;
    }
    int capY = topMargin + chartHeight - static_cast<int>((avgCap / maxHours) * chartHeight);
    painter.setPen(QPen(QColor("#F59E0B"), 1.5, Qt::DashLine));
    painter.drawLine(leftMargin, capY, w - rightMargin, capY);

    // Label for capacity line
    painter.setPen(QColor("#D97706"));
    painter.setFont(QFont("Segoe UI", 8, QFont::Bold));
    painter.drawText(w - rightMargin - 95, capY - 14, 90, 12, Qt::AlignRight, QString("Limit (%1h)").arg(avgCap, 0, 'f', 1));

    // Draw Bars
    int count = m_data.size();
    double colWidth = static_cast<double>(chartWidth) / count;
    double barWidth = std::min(38.0, colWidth * 0.65);

    for (int i = 0; i < count; ++i) {
        const auto& d = m_data[i];
        double barHeight = (d.scheduledHours / maxHours) * chartHeight;
        int x = leftMargin + static_cast<int>(i * colWidth + (colWidth - barWidth) / 2.0);
        int y = topMargin + chartHeight - static_cast<int>(barHeight);

        QRect barRect(x, y, static_cast<int>(barWidth), static_cast<int>(barHeight));

        // Color selection
        QColor barColor;
        if (d.isOverloaded) {
            barColor = (i == m_hoveredIndex) ? QColor("#DC2626") : QColor("#EF4444"); // Red for overloaded
        } else if (d.scheduledHours >= d.capacityHours * 0.8) {
            barColor = (i == m_hoveredIndex) ? QColor("#D97706") : QColor("#F59E0B"); // Amber near capacity
        } else {
            barColor = (i == m_hoveredIndex) ? QColor("#4338CA") : QColor("#6366F1"); // Vibrant indigo
        }

        // Draw bar with top rounded corners
        painter.setBrush(barColor);
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(barRect, 5, 5);

        // Draw value label above bar
        if (d.scheduledHours > 0.1) {
            painter.setPen(d.isOverloaded ? QColor("#DC2626") : QColor("#475569"));
            painter.setFont(QFont("Segoe UI", 9, QFont::Bold));
            painter.drawText(QRect(x - 10, y - 18, static_cast<int>(barWidth) + 20, 16),
                             Qt::AlignCenter, QString::number(d.scheduledHours, 'f', 1) + "h");
        }

        // Draw Day & Date labels below
        painter.setFont(QFont("Segoe UI", 9, (i == m_hoveredIndex) ? QFont::Bold : QFont::Normal));
        painter.setPen((i == m_hoveredIndex) ? QColor("#0F172A") : QColor("#64748B"));
        painter.drawText(QRect(x - 15, topMargin + chartHeight + 6, static_cast<int>(barWidth) + 30, 16),
                         Qt::AlignCenter, d.dayName);

        painter.setFont(QFont("Segoe UI", 8));
        painter.setPen(QColor("#94A3B8"));
        painter.drawText(QRect(x - 15, topMargin + chartHeight + 22, static_cast<int>(barWidth) + 30, 14),
                         Qt::AlignCenter, d.date.toString("M/d"));
    }
}
