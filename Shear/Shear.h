#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_Shear.h"

class Shear : public QMainWindow
{
    Q_OBJECT

public:
    Shear(QWidget *parent = nullptr);
    ~Shear();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots: 
    void onBrowseClicked(); 
    void onPathEditingFinished();


private:
    Ui::ShearClass ui;

    void loadSettings();
    void saveSettings();
};

