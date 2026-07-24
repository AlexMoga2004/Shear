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
#include <TrimSlider.h>

class TrimmerDialog : public QDialog
{
    Q_OBJECT

public:
    TrimmerDialog(const QString& videoPath, const QString& baseScanDir, QWidget* parent = nullptr);
    ~TrimmerDialog();

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
	QPushButton* m_btnRewind;
    QPushButton* m_btnPlay;
    QPushButton* m_btnForward;
	QPushButton* m_btnSetStartHere;
    QPushButton* m_btnSetEndHere;

    qint64 m_startTime = 0;
    qint64 m_endTime = 0;
    qint64 m_totalDuration = 0;

    QString formatTime(qint64 ms);
    void updateMarkerLabel();
};