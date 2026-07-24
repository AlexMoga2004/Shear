#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLineEdit>
#include <QFileDialog>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QTabWidget>
#include <QPushButton>
#include <QDir>

KeybindButton::KeybindButton(const QString& settingKey, QWidget* parent)
    : QPushButton(parent), m_settingKey(settingKey), m_isListening(false)
{
    m_currentKey = AppSettings::get().value(settingKey).toInt();
    updateText();
    connect(this, &QPushButton::clicked, this, &KeybindButton::onClicked);
}

void KeybindButton::onClicked() {
    m_isListening = true;
    setText("[ Press any key... ]");
    setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold;");
}

void KeybindButton::keyPressEvent(QKeyEvent* event) {
    if (!m_isListening) {
        QPushButton::keyPressEvent(event);
        return;
    }

    int key = event->key();
    if (key == Qt::Key_Shift || key == Qt::Key_Control || key == Qt::Key_Alt || key == Qt::Key_Meta) return;

    if (key != Qt::Key_Escape) {
        m_currentKey = key;
    }

    m_isListening = false;
    updateText();
}

void KeybindButton::focusOutEvent(QFocusEvent* event) {
    if (m_isListening) {
        m_isListening = false;
        updateText();
    }
    QPushButton::focusOutEvent(event);
}

void KeybindButton::updateText() {
    setText(QKeySequence(m_currentKey).toString());
    setStyleSheet("");
}

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Preferences");
    resize(400, 500);

    AppSettings::initDefaults();

    // --- VIDEO & OUTPUT TAB ---
    QWidget* tabVideo = new QWidget();
    QFormLayout* videoLayout = new QFormLayout(tabVideo);

    m_spinFps = new QSpinBox(this);
    m_spinFps->setRange(1, 240);
    m_spinFps->setValue(AppSettings::get().value("fps").toInt());

    m_spinPlaybackSpeed = new QDoubleSpinBox(this);
    m_spinPlaybackSpeed->setRange(0.1, 5.0);
    m_spinPlaybackSpeed->setSingleStep(0.1);
    m_spinPlaybackSpeed->setValue(AppSettings::get().value("playback_speed", 1.0).toDouble());

    m_spinRenderSpeed = new QDoubleSpinBox(this);
    m_spinRenderSpeed->setRange(0.1, 5.0);
    m_spinRenderSpeed->setSingleStep(0.1);
    m_spinRenderSpeed->setValue(AppSettings::get().value("render_speed", 1.0).toDouble());

    m_chkMaxSize = new QCheckBox("Target Max Size:", this);
    m_chkMaxSize->setToolTip("If unchecked, rendering will maintain the original video bitrate.");
    m_chkMaxSize->setChecked(AppSettings::get().value("limit_size").toBool());

    m_spinMaxSize = new QSpinBox(this);
    m_spinMaxSize->setRange(1, 10000);
    m_spinMaxSize->setSuffix(" MB");
    m_spinMaxSize->setValue(AppSettings::get().value("max_size_mb").toInt());
    m_spinMaxSize->setEnabled(m_chkMaxSize->isChecked());
    connect(m_chkMaxSize, &QCheckBox::toggled, m_spinMaxSize, &QSpinBox::setEnabled);

    m_comboResolution = new QComboBox(this);
    m_comboResolution->addItem("Original", 0);
    m_comboResolution->addItem("1080p", 1080);
    m_comboResolution->addItem("720p", 720);
    m_comboResolution->addItem("480p", 480);

    int savedRes = AppSettings::get().value("resolution").toInt();
    int index = m_comboResolution->findData(savedRes);
    if (index != -1) m_comboResolution->setCurrentIndex(index);

    m_lineSaveDir = new QLineEdit(this);
    m_lineSaveDir->setText(AppSettings::get().value("save_dir").toString());
    m_lineSaveDir->setPlaceholderText("Leave blank to save next to original file");

    m_btnBrowseSaveDir = new QPushButton("Browse...", this);
    connect(m_btnBrowseSaveDir, &QPushButton::clicked, this, &SettingsDialog::onBrowseSaveDirClicked);

    QHBoxLayout* dirLayout = new QHBoxLayout();
    dirLayout->addWidget(m_lineSaveDir);
    dirLayout->addWidget(m_btnBrowseSaveDir);

    // --- NEW PADDING CONTROLS ---
    m_spinStartPadding = new QSpinBox(this);
    m_spinStartPadding->setRange(0, 30);
    m_spinStartPadding->setSuffix(" sec");
    m_spinStartPadding->setValue(AppSettings::get().value("start_padding_sec", 0).toInt());

    m_spinEndPadding = new QSpinBox(this);
    m_spinEndPadding->setRange(0, 30);
    m_spinEndPadding->setSuffix(" sec");
    m_spinEndPadding->setValue(AppSettings::get().value("end_padding_sec", 0).toInt());

    // Populate Video Tab layout
    videoLayout->addRow("Default Save Folder:", dirLayout);
    videoLayout->addRow("Output Resolution:", m_comboResolution);
    videoLayout->addRow("Output FPS:", m_spinFps);
    videoLayout->addRow("UI Playback Speed:", m_spinPlaybackSpeed);
    videoLayout->addRow("Final Render Speed:", m_spinRenderSpeed);
    videoLayout->addRow(m_chkMaxSize, m_spinMaxSize);
    videoLayout->addRow(new QLabel(""), new QLabel("")); // Spacer row
    videoLayout->addRow(new QLabel("<b>Marker Keybind Padding:</b>"));
    videoLayout->addRow("Start Pre-roll (I):", m_spinStartPadding);
    videoLayout->addRow("End Post-roll (O):", m_spinEndPadding);

    // --- MAIN WINDOW BINDS TAB ---
    QWidget* tabMainBinds = new QWidget();
    QFormLayout* mainBindsLayout = new QFormLayout(tabMainBinds);

    m_btnNavLeft = new KeybindButton("key_nav_left", this);
    m_btnNavDown = new KeybindButton("key_nav_down", this);
    m_btnNavUp = new KeybindButton("key_nav_up", this);
    m_btnNavRight = new KeybindButton("key_nav_right", this);

    m_btnMainPrev = new KeybindButton("key_prev_page", this);
    m_btnMainNext = new KeybindButton("key_next_page", this);

    mainBindsLayout->addRow(new QLabel("<b>Vim Navigation (H J K L):</b>"));
    mainBindsLayout->addRow("Left:", m_btnNavLeft);
    mainBindsLayout->addRow("Down:", m_btnNavDown);
    mainBindsLayout->addRow("Up:", m_btnNavUp);
    mainBindsLayout->addRow("Right:", m_btnNavRight);

    mainBindsLayout->addRow(new QLabel(""), new QLabel(""));
    mainBindsLayout->addRow(new QLabel("<b>Page Navigation:</b>"));
    mainBindsLayout->addRow(new QLabel("<i>Note: Arrow keys also work for navigation.</i>"));
    mainBindsLayout->addRow("Previous Page:", m_btnMainPrev);
    mainBindsLayout->addRow("Next Page:", m_btnMainNext);

    // --- TRIMMER BINDS TAB ---
    QWidget* tabTrimmerBinds = new QWidget();
    QFormLayout* trimmerLayout = new QFormLayout(tabTrimmerBinds);

    m_btnStart = new KeybindButton("key_start", this);
    m_btnEnd = new KeybindButton("key_end", this);
    m_btnPlay = new KeybindButton("key_play", this);
    m_btnRewind = new KeybindButton("key_rewind", this);
    m_btnForward = new KeybindButton("key_forward", this);
    m_btnRender = new KeybindButton("key_render", this);
    m_btnCancel = new KeybindButton("key_cancel", this);

    trimmerLayout->addRow("Set Start Marker:", m_btnStart);
    trimmerLayout->addRow("Set End Marker:", m_btnEnd);
    trimmerLayout->addRow("Play / Pause:", m_btnPlay);
    trimmerLayout->addRow("Skip Back 5s:", m_btnRewind);
    trimmerLayout->addRow("Skip Forward 5s:", m_btnForward);
    trimmerLayout->addRow("Render Slices:", m_btnRender);
    trimmerLayout->addRow("Cancel Workspace:", m_btnCancel);

    // --- ASSEMBLE TABS ---
    QTabWidget* tabWidget = new QTabWidget(this);
    tabWidget->addTab(tabVideo, "Video & Output");
    tabWidget->addTab(tabMainBinds, "Main Binds");
    tabWidget->addTab(tabTrimmerBinds, "Trimmer Binds");

    QPushButton* btnSave = new QPushButton("Save Preferences", this);
    btnSave->setStyleSheet("background-color: #2196F3; color: white; padding: 8px; font-weight: bold;");
    connect(btnSave, &QPushButton::clicked, this, &SettingsDialog::onSaveClicked);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(tabWidget);
    mainLayout->addWidget(btnSave);
}

void SettingsDialog::onSaveClicked() {
    auto& settings = AppSettings::get();
    settings.setValue("fps", m_spinFps->value());
    settings.setValue("playback_speed", m_spinPlaybackSpeed->value());
    settings.setValue("render_speed", m_spinRenderSpeed->value());
    settings.setValue("limit_size", m_chkMaxSize->isChecked());
    settings.setValue("max_size_mb", m_spinMaxSize->value());
    settings.setValue("resolution", m_comboResolution->currentData().toInt());
    settings.setValue("save_dir", m_lineSaveDir->text());

    // Save padding values
    settings.setValue("start_padding_sec", m_spinStartPadding->value());
    settings.setValue("end_padding_sec", m_spinEndPadding->value());

    settings.setValue("key_nav_left", m_btnNavLeft->currentKey());
    settings.setValue("key_nav_down", m_btnNavDown->currentKey());
    settings.setValue("key_nav_up", m_btnNavUp->currentKey());
    settings.setValue("key_nav_right", m_btnNavRight->currentKey());

    settings.setValue("key_start", m_btnStart->currentKey());
    settings.setValue("key_end", m_btnEnd->currentKey());
    settings.setValue("key_play", m_btnPlay->currentKey());
    settings.setValue("key_rewind", m_btnRewind->currentKey());
    settings.setValue("key_forward", m_btnForward->currentKey());
    settings.setValue("key_render", m_btnRender->currentKey());
    settings.setValue("key_cancel", m_btnCancel->currentKey());

    accept();
}

void SettingsDialog::onBrowseSaveDirClicked() {
    QString currentDir = m_lineSaveDir->text().isEmpty() ? "C:/" : m_lineSaveDir->text();

    QString dir = QFileDialog::getExistingDirectory(this, "Select Default Save Directory",
        currentDir, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!dir.isEmpty()) {
        m_lineSaveDir->setText(QDir::toNativeSeparators(dir));
    }
}