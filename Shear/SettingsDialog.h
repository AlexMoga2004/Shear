#pragma once

#include <QDialog>
#include <QSettings>
#include <QPushButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QKeyEvent>
#include <QFormLayout>

// 1. Global Helper for instant access anywhere in the app
class AppSettings {
public:
    static QSettings& get() {
        // Saves to: Computer\HKEY_CURRENT_USER\SOFTWARE\ShearApp\Settings
        static QSettings settings("ShearApp", "Settings");
        return settings;
    }

    static void initDefaults() {
        if (!get().contains("fps")) get().setValue("fps", 60);
        if (!get().contains("speed")) get().setValue("speed", 1.0);

        // Default Keybinds
        if (!get().contains("key_start")) get().setValue("key_start", Qt::Key_I);
        if (!get().contains("key_end")) get().setValue("key_end", Qt::Key_O);
        if (!get().contains("key_play")) get().setValue("key_play", Qt::Key_Space);
        if (!get().contains("key_rewind")) get().setValue("key_rewind", Qt::Key_J);
        if (!get().contains("key_forward")) get().setValue("key_forward", Qt::Key_L);
        if (!get().contains("key_render")) get().setValue("key_render", Qt::Key_Return);
        if (!get().contains("key_cancel")) get().setValue("key_cancel", Qt::Key_Backspace);
    }
};

// 2. Custom Button to capture keystrokes
class KeybindButton : public QPushButton {
    Q_OBJECT
public:
    KeybindButton(const QString& settingKey, QWidget* parent = nullptr);
    int currentKey() const { return m_currentKey; }

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

private slots:
    void onClicked();

private:
    QString m_settingKey;
    int m_currentKey;
    bool m_isListening;
    void updateText();
};

// 3. The Main Settings Window
class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    SettingsDialog(QWidget* parent = nullptr);

private slots:
    void onSaveClicked();

private:
    QSpinBox* m_spinFps;
    QDoubleSpinBox* m_spinSpeed;

    KeybindButton* m_btnStart;
    KeybindButton* m_btnEnd;
    KeybindButton* m_btnPlay;
    KeybindButton* m_btnRewind;
    KeybindButton* m_btnForward;
    KeybindButton* m_btnRender;
    KeybindButton* m_btnCancel;
};

