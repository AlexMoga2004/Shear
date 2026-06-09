#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_Shear.h"

class Shear : public QMainWindow
{
    Q_OBJECT

public:
    Shear(QWidget *parent = nullptr);
    ~Shear();

private slots: 
    void onBrowseClicked(); 
    void onPathEditingFinished();

private:
    Ui::ShearClass ui;
};

