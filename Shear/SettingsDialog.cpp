#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeySequence>

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
    resize(350, 400);

    AppSettings::initDefaults();

    m_spinFps = new QSpinBox(this);
    m_spinFps->setRange(1, 240);
    m_spinFps->setValue(AppSettings::get().value("fps").toInt());

    m_spinSpeed = new QDoubleSpinBox(this);
    m_spinSpeed->setRange(0.1, 5.0);
    m_spinSpeed->setSingleStep(0.1);
    m_spinSpeed->setValue(AppSettings::get().value("speed").toDouble());

    m_chkMaxSize = new QCheckBox("Target Max Size:", this);
    m_chkMaxSize->setToolTip("If unchecked, rendering will maintain the original video bitrate.");
    m_chkMaxSize->setChecked(AppSettings::get().value("limit_size").toBool());

    m_spinMaxSize = new QSpinBox(this);
    m_spinMaxSize->setRange(1, 10000); // 1MB to 10GB
    m_spinMaxSize->setSuffix(" MB");
    m_spinMaxSize->setValue(AppSettings::get().value("max_size_mb").toInt());

    // Immediately gray out the spinbox if the checkbox is not checked
    m_spinMaxSize->setEnabled(m_chkMaxSize->isChecked());

    // Wire them together so clicking the checkbox enables/disables the spinbox live
    connect(m_chkMaxSize, &QCheckBox::toggled, m_spinMaxSize, &QSpinBox::setEnabled);

    m_btnStart = new KeybindButton("key_start", this);
    m_btnEnd = new KeybindButton("key_end", this);
    m_btnPlay = new KeybindButton("key_play", this);
    m_btnRewind = new KeybindButton("key_rewind", this);
    m_btnForward = new KeybindButton("key_forward", this);
    m_btnRender = new KeybindButton("key_render", this);
    m_btnCancel = new KeybindButton("key_cancel", this);

    QFormLayout* formLayout = new QFormLayout();
    formLayout->addRow("Output FPS:", m_spinFps);
    formLayout->addRow("Playback Speed:", m_spinSpeed);
    formLayout->addRow(m_chkMaxSize, m_spinMaxSize);
    formLayout->addRow(new QLabel(""), new QLabel("")); // Spacer
    formLayout->addRow("Set Start Marker:", m_btnStart);
    formLayout->addRow("Set End Marker:", m_btnEnd);
    formLayout->addRow("Play / Pause:", m_btnPlay);
    formLayout->addRow("Skip Back 5s:", m_btnRewind);
    formLayout->addRow("Skip Forward 5s:", m_btnForward);
    formLayout->addRow("Render Slices:", m_btnRender);
    formLayout->addRow("Cancel Workspace:", m_btnCancel);

    QPushButton* btnSave = new QPushButton("Save Preferences", this);
    btnSave->setStyleSheet("background-color: #2196F3; color: white; padding: 8px; font-weight: bold;");
    connect(btnSave, &QPushButton::clicked, this, &SettingsDialog::onSaveClicked);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addStretch();
    mainLayout->addWidget(btnSave);
}

void SettingsDialog::onSaveClicked() {
    auto& settings = AppSettings::get();
    settings.setValue("fps", m_spinFps->value());
    settings.setValue("speed", m_spinSpeed->value());
    settings.setValue("limit_size", m_chkMaxSize->isChecked());
    settings.setValue("max_size_mb", m_spinMaxSize->value());

    settings.setValue("key_start", m_btnStart->currentKey());
    settings.setValue("key_end", m_btnEnd->currentKey());
    settings.setValue("key_play", m_btnPlay->currentKey());
    settings.setValue("key_rewind", m_btnRewind->currentKey());
    settings.setValue("key_forward", m_btnForward->currentKey());
    settings.setValue("key_render", m_btnRender->currentKey());
    settings.setValue("key_cancel", m_btnCancel->currentKey());

    accept();
}