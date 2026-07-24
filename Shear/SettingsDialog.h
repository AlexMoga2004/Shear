#pragma once

#include <QDialog>
#include <QSettings>
#include <QPushButton>
#include <QSpinBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QKeyEvent>
#include <QComboBox>
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

        if (!get().contains("limit_size")) get().setValue("limit_size", false);
        if (!get().contains("max_size_mb")) get().setValue("max_size_mb", 10); // 10MB is the discord limit

        if (!get().contains("save_dir")) get().setValue("save_dir", "");
        if (!get().contains("resolution")) get().setValue("resolution", 0);

        if (!get().contains("key_start")) get().setValue("key_start", Qt::Key_I);
        if (!get().contains("key_end")) get().setValue("key_end", Qt::Key_O);
        if (!get().contains("key_play")) get().setValue("key_play", Qt::Key_Space);
        if (!get().contains("key_rewind")) get().setValue("key_rewind", Qt::Key_J);
        if (!get().contains("key_forward")) get().setValue("key_forward", Qt::Key_L);
        if (!get().contains("key_render")) get().setValue("key_render", Qt::Key_Return);
        if (!get().contains("key_cancel")) get().setValue("key_cancel", Qt::Key_Backspace);

        if (!get().contains("key_nav_left")) get().setValue("key_nav_left", Qt::Key_H);
        if (!get().contains("key_nav_down")) get().setValue("key_nav_down", Qt::Key_J);
        if (!get().contains("key_nav_up")) get().setValue("key_nav_up", Qt::Key_K);
        if (!get().contains("key_nav_right")) get().setValue("key_nav_right", Qt::Key_L);

        if (!get().contains("key_prev_page")) get().setValue("key_prev_page", Qt::Key_Comma);
        if (!get().contains("key_next_page")) get().setValue("key_next_page", Qt::Key_Period);

        if (!get().contains("key_prev_page")) get().setValue("key_prev_page", Qt::Key_Comma);
        if (!get().contains("key_next_page")) get().setValue("key_next_page", Qt::Key_Period);
    }
};

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

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    SettingsDialog(QWidget* parent = nullptr);
    void onBrowseSaveDirClicked();

private slots:
    void onSaveClicked();

private:
    QSpinBox* m_spinFps;
    QDoubleSpinBox* m_spinPlaybackSpeed;
    QDoubleSpinBox* m_spinRenderSpeed;
    QCheckBox* m_chkMaxSize;
    QSpinBox* m_spinMaxSize;
    QLineEdit* m_lineSaveDir;
    QPushButton* m_btnBrowseSaveDir;
    QComboBox* m_comboResolution;
    QSpinBox* m_spinStartPadding;
    QSpinBox* m_spinEndPadding;

    KeybindButton* m_btnStart;
    KeybindButton* m_btnEnd;
    KeybindButton* m_btnPlay;
    KeybindButton* m_btnRewind;
    KeybindButton* m_btnForward;
    KeybindButton* m_btnRender;
    KeybindButton* m_btnCancel;
    KeybindButton* m_btnNavLeft;
    KeybindButton* m_btnNavRight;
    KeybindButton* m_btnNavUp;
    KeybindButton* m_btnNavDown;
    KeybindButton* m_btnMainPrev;
    KeybindButton* m_btnMainNext;
};

