#include "TrimmerDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTime>
#include <QMessageBox>

TrimmerDialog::TrimmerDialog(const QString& videoPath, QWidget* parent)
    : QDialog(parent), m_videoPath(videoPath)
{
    setWindowTitle("Trim Workspace - " + videoPath);
    resize(800, 600);

    m_player = new QMediaPlayer(this);
    m_videoWidget = new QVideoWidget(this);
    m_audioOutput = new QAudioOutput(this);

    m_player->setVideoOutput(m_videoWidget);
    m_player->setAudioOutput(m_audioOutput);

    m_timelineSlider = new TrimSlider(Qt::Horizontal, this);
    m_lblCurrentTime = new QLabel("00:00:00", this);
    m_lblDuration = new QLabel("00:00:00", this);
    m_lblMarkers = new QLabel("Trim Window: [ Not Set ] to [ Not Set ]", this);

    m_btnRender = new QPushButton("RENDER (Enter)", this);
    m_btnCancel = new QPushButton("CANCEL (Backspace)", this);

    m_timelineSlider->setFocusPolicy(Qt::NoFocus);
    m_videoWidget->setFocusPolicy(Qt::NoFocus);
    m_btnRender->setFocusPolicy(Qt::NoFocus);
    m_btnCancel->setFocusPolicy(Qt::NoFocus);

    m_lblMarkers->setStyleSheet("font-weight: bold; color: #4CAF50; font-size: 14px;");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    mainLayout->addWidget(m_videoWidget, 1);

    QHBoxLayout* timelineLayout = new QHBoxLayout();
    timelineLayout->addWidget(m_lblCurrentTime);
    timelineLayout->addWidget(m_timelineSlider);
    timelineLayout->addWidget(m_lblDuration);
    mainLayout->addLayout(timelineLayout);

    mainLayout->addWidget(m_lblMarkers, 0, Qt::AlignCenter);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_btnRender);
    buttonLayout->addWidget(m_btnCancel);
    mainLayout->addLayout(buttonLayout);

    connect(m_player, &QMediaPlayer::positionChanged, this, &TrimmerDialog::onPositionChanged);
    connect(m_player, &QMediaPlayer::durationChanged, this, &TrimmerDialog::onDurationChanged);
    connect(m_timelineSlider, &QSlider::sliderMoved, this, &TrimmerDialog::sliderMoved);

    connect(m_btnRender, &QPushButton::clicked, this, &TrimmerDialog::triggerRender);
    connect(m_btnCancel, &QPushButton::clicked, this, &TrimmerDialog::reject);

    m_player->setSource(QUrl::fromLocalFile(videoPath));
    m_player->play();
}

TrimmerDialog::~TrimmerDialog() {
    m_player->stop();
}

void TrimmerDialog::onDurationChanged(qint64 duration) {
    m_totalDuration = duration;
    m_timelineSlider->setRange(0, duration);
    m_lblDuration->setText(formatTime(duration));
    m_endTime = duration; 
    updateMarkerLabel();
}

void TrimmerDialog::onPositionChanged(qint64 position) {
    if (!m_timelineSlider->isSliderDown()) {
        m_timelineSlider->setValue(position);
    }
    m_lblCurrentTime->setText(formatTime(position));
}

void TrimmerDialog::sliderMoved(int position) {
    m_player->setPosition(position);
}

QString TrimmerDialog::formatTime(qint64 ms) {
    QTime time(0, 0, 0);
    time = time.addMSecs(ms);
    return time.toString("hh:mm:ss");
}

void TrimmerDialog::updateMarkerLabel() {
    m_lblMarkers->setText(QString("Trim Selection: [ %1 ] ➔ [ %2 ]")
        .arg(formatTime(m_startTime))
        .arg(formatTime(m_endTime)));

    m_timelineSlider->setMarkers(m_startTime, m_endTime);
}

void TrimmerDialog::setStartMarker() {
    m_startTime = m_player->position();
    if (m_startTime > m_endTime) m_endTime = m_totalDuration;
    updateMarkerLabel();

    m_player->setPosition(m_startTime);
}

void TrimmerDialog::setEndMarker() {
    m_endTime = m_player->position();
    if (m_endTime < m_startTime) m_startTime = 0;
    updateMarkerLabel();
}

void TrimmerDialog::togglePlayPause() {
    if (m_player->playbackState() == QMediaPlayer::PlayingState) {
        m_player->pause();

        if (m_startTime > 0) {
            m_player->setPosition(m_startTime);
        }
    }
    else {
        m_player->play();
    }
}

void TrimmerDialog::triggerRender() {
    // TODO: ffmpeg functionality
    QMessageBox::information(this, "Render Request",
        QString("Sending slice arguments to FFmpeg:\nStart: %1\nEnd: %2")
        .arg(formatTime(m_startTime)).arg(formatTime(m_endTime)));
    accept();
}

void TrimmerDialog::keyPressEvent(QKeyEvent* event)
{
    qint64 currentPos = m_player->position();
    qint64 newPos = currentPos;

    switch (event->key()) {
    case Qt::Key_I:
        setStartMarker();
        break;
    case Qt::Key_O:
        setEndMarker();
        break;

    case Qt::Key_Space:
    case Qt::Key_K: 
        togglePlayPause();
        break;

    case Qt::Key_J:
    case Qt::Key_Left:
        newPos = qMax(0LL, currentPos - 5000); 
        m_player->setPosition(newPos);
        break;

    case Qt::Key_L:
    case Qt::Key_Right:
        newPos = qMin(m_totalDuration, currentPos + 5000); 
        m_player->setPosition(newPos);
        break;

    case Qt::Key_Return:
    case Qt::Key_Enter:
        triggerRender();
        break;
    case Qt::Key_Backspace:
        reject();
        break;

    default:
        QDialog::keyPressEvent(event);
    }
}