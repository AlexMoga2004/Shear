#include "TrimSlider.h"
#include <QPainter>
#include <QStyle>

void TrimSlider::paintEvent(QPaintEvent* event) {
    QSlider::paintEvent(event);

    if (maximum() <= 0) return;

    QPainter painter(this);

    int startX = QStyle::sliderPositionFromValue(minimum(), maximum(), m_start, width());
    int endX = QStyle::sliderPositionFromValue(minimum(), maximum(), m_end, width());

    painter.setPen(QPen(QColor("#4CAF50"), 3));
    painter.drawLine(startX, 0, startX, height());

    painter.setPen(QPen(QColor("#F44336"), 3));
    painter.drawLine(endX, 0, endX, height());
}