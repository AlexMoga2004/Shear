#include "TrimmerDialog.h"
#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTime>
#include <QDateTime>
#include <Qt>
#include <QMessageBox>

#include <QProcess>
#include <QProgressDialog>
#include <QFileInfo>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QDebug>
#include <QClipboard>
#include <QMimeData>

#include <QShortcut>
#include <QKeySequence>

TrimmerDialog::TrimmerDialog(const QString& videoPath, const QString& baseScanDir, QWidget* parent)
    : QDialog(parent), m_videoPath(videoPath), m_baseScanDir(baseScanDir)
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

    // Playback and Marker Snapping Buttons
    m_btnRewind = new QPushButton("⏪ -5s", this);
    m_btnPlay = new QPushButton("▶ / ⏸", this);
    m_btnForward = new QPushButton("+5s ⏩", this);
    m_btnSetStartHere = new QPushButton("📍 Set Start Here", this);
    m_btnSetEndHere = new QPushButton("📍 Set End Here", this);

    QString ctrlStyle = "QPushButton { padding: 6px 12px; font-weight: bold; }";
    m_btnRewind->setStyleSheet(ctrlStyle);
    m_btnPlay->setStyleSheet(ctrlStyle);
    m_btnForward->setStyleSheet(ctrlStyle);
    m_btnSetStartHere->setStyleSheet(ctrlStyle + " QPushButton { color: green; }");
    m_btnSetEndHere->setStyleSheet(ctrlStyle + " QPushButton { color: red; }");

    m_lblMarkers->setStyleSheet("font-weight: bold; color: #4CAF50; font-size: 14px;");

    m_player->setPlaybackRate(AppSettings::get().value("playback_speed", 1.0).toDouble());

    auto& settings = AppSettings::get();

    // Setup keyboard shortcuts within the trimmer dialogue window

    // Start Marker
    QShortcut* cutStart = new QShortcut(QKeySequence(settings.value("key_start").toInt()), this);
    connect(cutStart, &QShortcut::activated, this, &TrimmerDialog::setStartMarker);

    // End Marker
    QShortcut* cutEnd = new QShortcut(QKeySequence(settings.value("key_end").toInt()), this);
    connect(cutEnd, &QShortcut::activated, this, &TrimmerDialog::setEndMarker);

    // Play / Pause
    QShortcut* cutPlay = new QShortcut(QKeySequence(settings.value("key_play").toInt()), this);
    connect(cutPlay, &QShortcut::activated, this, &TrimmerDialog::togglePlayPause);

    // Rewind 
    auto rewindFunc = [this]() {
        m_player->setPosition(qMax(0LL, m_player->position() - 5000));
        };
    QShortcut* cutRewind = new QShortcut(QKeySequence(settings.value("key_rewind").toInt()), this);
    QShortcut* cutRewindAlt = new QShortcut(QKeySequence(Qt::Key_Left), this);
    connect(cutRewind, &QShortcut::activated, this, rewindFunc);
    connect(cutRewindAlt, &QShortcut::activated, this, rewindFunc);

    // Forward 
    auto forwardFunc = [this]() {
        m_player->setPosition(qMin(m_totalDuration, m_player->position() + 5000));
        };
    QShortcut* cutForward = new QShortcut(QKeySequence(settings.value("key_forward").toInt()), this);
    QShortcut* cutForwardAlt = new QShortcut(QKeySequence(Qt::Key_Right), this);
    connect(cutForward, &QShortcut::activated, this, forwardFunc);
    connect(cutForwardAlt, &QShortcut::activated, this, forwardFunc);

    // Render
    QShortcut* cutRender = new QShortcut(QKeySequence(settings.value("key_render").toInt()), this);
    connect(cutRender, &QShortcut::activated, this, &TrimmerDialog::triggerRender);

    // Cancel
    QShortcut* cutCancel = new QShortcut(QKeySequence(settings.value("key_cancel").toInt()), this);
    connect(cutCancel, &QShortcut::activated, this, &TrimmerDialog::reject);

    // --- Layout Building ---
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    mainLayout->addWidget(m_videoWidget, 1);

    // Timeline row: Current Time -> Timeline Slider -> Duration
    QHBoxLayout* timelineLayout = new QHBoxLayout();
    timelineLayout->addWidget(m_lblCurrentTime);
    timelineLayout->addWidget(m_timelineSlider, 1);
    timelineLayout->addWidget(m_lblDuration);
    mainLayout->addLayout(timelineLayout);

    // Control panel row underneath the timeline: Playback controls + Snapping buttons
    QHBoxLayout* controlRowLayout = new QHBoxLayout();
    controlRowLayout->addWidget(m_btnRewind);
    controlRowLayout->addWidget(m_btnPlay);
    controlRowLayout->addWidget(m_btnForward);
    controlRowLayout->addStretch();
    controlRowLayout->addWidget(m_btnSetStartHere);
    controlRowLayout->addWidget(m_btnSetEndHere);
    mainLayout->addLayout(controlRowLayout);

    mainLayout->addWidget(m_lblMarkers, 0, Qt::AlignCenter);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_btnRender);
    buttonLayout->addWidget(m_btnCancel);
    mainLayout->addLayout(buttonLayout);

    // --- Signal Connections ---
    connect(m_btnRewind, &QPushButton::clicked, this, [this]() {
        m_player->setPosition(qMax(0LL, m_player->position() - 5000));
        });
    connect(m_btnPlay, &QPushButton::clicked, this, &TrimmerDialog::togglePlayPause);
    connect(m_btnForward, &QPushButton::clicked, this, [this]() {
        m_player->setPosition(qMin(m_totalDuration, m_player->position() + 5000));
        });

    connect(m_btnSetStartHere, &QPushButton::clicked, this, [this]() {
        qint64 currentPos = m_player->position();
        if (currentPos < m_endTime) {
            m_startTime = currentPos;
            updateMarkerLabel();
        }
        });

    connect(m_btnSetEndHere, &QPushButton::clicked, this, [this]() {
        qint64 currentPos = m_player->position();
        if (currentPos > m_startTime) {
            m_endTime = currentPos;
            updateMarkerLabel();
        }
        });

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
    int paddingSec = AppSettings::get().value("start_padding_sec", 0).toInt();
    qint64 paddedPos = m_player->position() - (paddingSec * 1000LL);
    m_startTime = qMax(0LL, paddedPos);

    if (m_startTime > m_endTime) m_endTime = m_totalDuration;
    updateMarkerLabel();

    m_player->setPosition(m_startTime);
}

void TrimmerDialog::setEndMarker() {
    int paddingSec = AppSettings::get().value("end_padding_sec", 0).toInt();
    qint64 paddedPos = m_player->position() + (paddingSec * 1000LL);
    m_endTime = qMin(m_totalDuration, paddedPos);

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
    auto& settings = AppSettings::get();
    bool limitSize = settings.value("limit_size").toBool();
    int maxSizeMB = settings.value("max_size_mb").toInt();
    int fps = settings.value("fps").toInt();
    double speed = settings.value("render_speed", 1.0).toDouble();

    // 1. Calculate Timestamps
    double startSec = m_startTime / 1000.0;
    double originalDuration = (m_endTime - m_startTime) / 1000.0;
    double finalDuration = originalDuration / speed;

    if (finalDuration <= 0) {
        QMessageBox::warning(this, "Trim Error", "Invalid duration. Check your markers.");
        return;
    }

    // 2. Setup File Paths & Nested Folders
    int resolution = settings.value("resolution").toInt();

    QFileInfo inputInfo(m_videoPath);
    QString originalDir = inputInfo.absolutePath();
    QString baseName = inputInfo.completeBaseName();

    QDir baseDir(m_baseScanDir);
    QString relativeFolderTree = baseDir.relativeFilePath(originalDir);

    QString customSaveDir = settings.value("save_dir").toString();
    QString targetDir = originalDir;

    if (!customSaveDir.isEmpty() && QDir(customSaveDir).exists()) {
        if (relativeFolderTree == ".") {
            targetDir = customSaveDir;
        }
        else {
            targetDir = customSaveDir + "/" + relativeFolderTree;
        }

        QDir().mkpath(targetDir);
    }

    QString outPath;
    QString timeStamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");

    if (targetDir == originalDir) {
        outPath = targetDir + "/" + baseName + "_trimmed_" + timeStamp + ".mp4";
    }
    else {
        outPath = targetDir + "/" + baseName + ".mp4";
    }

    QString passLogPrefix = targetDir + "/ffmpeg_2pass_log";
    QString appDir = QCoreApplication::applicationDirPath();
    QString ffmpegExe = QDir(appDir).filePath("bin/ffmpeg/win/ffmpeg.exe");
    if (!QFileInfo::exists(ffmpegExe)) ffmpegExe = "ffmpeg";

    // 3. Build Base FFmpeg Arguments & Filters
    QStringList baseArgs;
    baseArgs << "-y"
        << "-ss" << QString::number(startSec, 'f', 3)
        << "-i" << m_videoPath
        << "-t" << QString::number(originalDuration, 'f', 3);

    QStringList filterArgs;
    QStringList vf;
    if (fps > 0) vf << QString("fps=%1").arg(fps);
    if (speed != 1.0) vf << QString("setpts=%1*PTS").arg(1.0 / speed);

    if (resolution > 0) {
        vf << QString("scale=-2:%1").arg(resolution);
    }

    if (!vf.isEmpty()) filterArgs << "-vf" << vf.join(",");
    if (speed != 1.0) filterArgs << "-af" << QString("atempo=%1").arg(speed);

    // 4. Calculate Bitrate Target with a 5% Safety Margin
    int videoBitrateK = 0;
    if (limitSize) {
        double safeTargetMB = maxSizeMB * 0.95;
        double totalKbps = (safeTargetMB * 8192.0) / finalDuration;
        videoBitrateK = qMax(50, (int)(totalKbps - 128));
    }

    // 5. Setup Indeterminate Loading UI
    QProgressDialog progress("Rendering Video...", "Cancel", 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setAutoClose(true);
    progress.show();

    auto runProcess = [&](const QStringList& args) -> bool {
        QProcess p;
        p.start(ffmpegExe, args);

        while (!p.waitForFinished(100)) {
            QCoreApplication::processEvents();
            if (progress.wasCanceled()) {
                p.kill();
                return false;
            }
        }
        return p.exitCode() == 0;
        };

    // 6. Execution Block
    if (!limitSize) {
        progress.setLabelText("Rendering High Quality Cut...");
        QStringList args = baseArgs + filterArgs;
        args << "-c:v" << "libx264" << "-crf" << "18"
            << "-c:a" << "aac" << "-b:a" << "128k"
            << outPath;

        if (!runProcess(args)) return;
    }
    else {
        int attempts = 0;
        bool fileIsTooBig = true;
        const int MAX_ATTEMPTS = 3;

        while (fileIsTooBig && attempts < MAX_ATTEMPTS) {
            attempts++;

            progress.setLabelText(QString("Analyzing Video (Attempt %1)...").arg(attempts));

            QStringList pass1Args = baseArgs + filterArgs;
            pass1Args << "-c:v" << "libx264" << "-b:v" << QString("%1k").arg(videoBitrateK)
                << "-pass" << "1" << "-passlogfile" << passLogPrefix
                << "-an" << "-f" << "mp4" << "NUL";

            if (!runProcess(pass1Args)) return;

            progress.setLabelText(QString("Writing Final Data (Attempt %1)...").arg(attempts));

            QStringList pass2Args = baseArgs + filterArgs;
            pass2Args << "-c:v" << "libx264"
                << "-b:v" << QString("%1k").arg(videoBitrateK)
                << "-maxrate" << QString("%1k").arg(videoBitrateK)
                << "-bufsize" << QString("%1k").arg(videoBitrateK * 2)
                << "-pass" << "2" << "-passlogfile" << passLogPrefix
                << "-c:a" << "aac" << "-b:a" << "128k"
                << outPath;

            if (!runProcess(pass2Args)) return;

            QFile finalFile(outPath);
            double actualMB = finalFile.size() / (1024.0 * 1024.0);

            if (actualMB <= maxSizeMB) {
                fileIsTooBig = false;
            }
            else {
                progress.setLabelText("Limit exceeded. Re-compressing...");
                videoBitrateK = (int)(videoBitrateK * 0.80);
            }
        }

        if (fileIsTooBig) {
            QMessageBox::warning(this, "Size Limit Failed",
                "Could not compress the file enough without destroying the video completely.\nTry a shorter clip.");
        }

        QFile::remove(passLogPrefix + "-0.log");
        QFile::remove(passLogPrefix + "-0.log.mbtree");
    }

    // 7. Success
    progress.close();

    QMimeData* mimeData = new QMimeData();
    mimeData->setUrls({ QUrl::fromLocalFile(outPath) });
    QGuiApplication::clipboard()->setMimeData(mimeData);

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Success");
    msgBox.setText("Video rendered successfully!\n\nIt has been copied to your clipboard. You can paste it directly into Discord.");

    QPushButton* btnOpenFolder = msgBox.addButton("Open Folder", QMessageBox::ActionRole);
    QPushButton* btnDeleteOrig = msgBox.addButton("Delete Original", QMessageBox::DestructiveRole);
    msgBox.addButton(QMessageBox::Ok);

    msgBox.exec();

    if (msgBox.clickedButton() == btnOpenFolder) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(targetDir));
    }
    else if (msgBox.clickedButton() == btnDeleteOrig) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::warning(this, "Permanent Delete",
            "Are you absolutely sure you want to delete the original raw footage?\n\nThis bypasses the Recycle Bin and CANNOT be undone.",
            QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            m_player->stop();
            m_player->setSource(QUrl());

            if (QFile::remove(m_videoPath)) {
                QMessageBox::information(this, "Deleted", "Original file removed.");
            }
            else {
                QMessageBox::critical(this, "Error", "Could not delete the file. It may be open in another program.");
            }
        }
    }

    accept();
}
