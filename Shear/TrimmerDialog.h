#pragma once

#include <QDialog>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QAudioOutput>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QKeyEvent>
#include <QPainter>
#include <QStyle>

class TrimSlider : public QSlider
{
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
    void paintEvent(QPaintEvent* event) override {
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

private:
    qint64 m_start;
    qint64 m_end;
};

class TrimmerDialog : public QDialog
{
    Q_OBJECT

public:
    TrimmerDialog(const QString& videoPath, const QString& baseScanDir, QWidget* parent = nullptr);
    ~TrimmerDialog();

protected:
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void onPositionChanged(qint64 position);
    void onDurationChanged(qint64 duration);
    void sliderMoved(int position);
    void togglePlayPause();
    void setStartMarker();
    void setEndMarker();
    void triggerRender();

private:
    QString m_videoPath;
    QString m_baseScanDir;

    QMediaPlayer* m_player;
    QVideoWidget* m_videoWidget;
    QAudioOutput* m_audioOutput;

    TrimSlider* m_timelineSlider; 
    QLabel* m_lblCurrentTime;
    QLabel* m_lblDuration;
    QLabel* m_lblMarkers;

    QPushButton* m_btnRender;
    QPushButton* m_btnCancel;

    qint64 m_startTime = 0;
    qint64 m_endTime = 0;
    qint64 m_totalDuration = 0;

    QString formatTime(qint64 ms);
    void updateMarkerLabel();
};