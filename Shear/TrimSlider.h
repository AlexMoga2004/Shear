#pragma once
#include <QSlider>

class TrimSlider : public QSlider {
    Q_OBJECT
public:
    TrimSlider(Qt::Orientation orientation, QWidget* parent = nullptr)
        : QSlider(orientation, parent), m_start(0), m_end(0) {
    }

    void setMarkers(qint64 start, qint64 end) {
        m_start = start;
        m_end = end;
        update();
    }

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    qint64 m_start;
    qint64 m_end;
};