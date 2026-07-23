#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLineEdit>
#include <QFileDialog>

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
    resize(400, 450); // Made slightly taller to accommodate tabs

    AppSettings::initDefaults(); // Make sure to add "playback_speed", "render_speed", "key_prev_page" (Qt::Key_H), and "key_next_page" (Qt::Key_L) to this!

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

    videoLayout->addRow("Default Save Folder:", dirLayout);
    videoLayout->addRow("Output Resolution:", m_comboResolution);
    videoLayout->addRow("Output FPS:", m_spinFps);
    videoLayout->addRow("UI Playback Speed:", m_spinPlaybackSpeed);
    videoLayout->addRow("Final Render Speed:", m_spinRenderSpeed);
    videoLayout->addRow(m_chkMaxSize, m_spinMaxSize);

    // --- MAIN WINDOW BINDS TAB ---
    QWidget* tabMainBinds = new QWidget();
    QFormLayout* mainBindsLayout = new QFormLayout(tabMainBinds);

    // Inside your SettingsDialog constructor, under the Main Binds Tab:

    m_btnNavLeft = new KeybindButton("key_nav_left", this);
    m_btnNavDown = new KeybindButton("key_nav_down", this);
    m_btnNavUp = new KeybindButton("key_nav_up", this);
    m_btnNavRight = new KeybindButton("key_nav_right", this);

    mainBindsLayout->addRow(new QLabel("<i>Vim Navigation (H J K L):</i>"));
    mainBindsLayout->addRow("Left:", m_btnNavLeft);
    mainBindsLayout->addRow("Down:", m_btnNavDown);
    mainBindsLayout->addRow("Up:", m_btnNavUp);
    mainBindsLayout->addRow("Right:", m_btnNavRight);

    mainBindsLayout->addRow(new QLabel("<i>Note: Arrow keys also work for navigation.</i>"));
    mainBindsLayout->addRow("Previous Page:", m_btnNavLeft);
    mainBindsLayout->addRow("Previous Page:", m_btnNavRight);
    mainBindsLayout->addRow("Previous Page:", m_btnNavUp);
    mainBindsLayout->addRow("Previous Page:", m_btnNavDown);

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
