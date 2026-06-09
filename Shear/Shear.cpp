#include "Shear.h"
#include <QFileDialog>  
#include <QDir>        
#include <QMessageBox> 
#include <QSettings>

Shear::Shear(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    ui.btnRefresh->setToolTip("Refresh directory");
    ui.btnSettings->setToolTip("Settings");
    ui.btnBrowse->setToolTip("Select folder");

    connect(ui.btnBrowse, &QPushButton::clicked, this, &Shear::onBrowseClicked);
    connect(ui.linePath, &QLineEdit::editingFinished, this, &Shear::onPathEditingFinished);

    loadSettings();
}

Shear::~Shear()
{
}

void Shear::onBrowseClicked()
{
    QString startDir = ui.linePath->text().isEmpty() ? "C:/" : ui.linePath->text();

    QString dir = QFileDialog::getExistingDirectory(this, "Select Video Directory",
        startDir,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        ui.linePath->setText(QDir::toNativeSeparators(dir));
    }
}

void Shear::onPathEditingFinished()
{
    QString path = ui.linePath->text();

    if (path.isEmpty()) return;

    QDir dir(path);
    if (!dir.exists()) {
        QMessageBox::warning(this, "Invalid Directory", "The directory you entered does not exist.\nPlease check for typos.");

        ui.linePath->setFocus();
        ui.linePath->selectAll();
    }
}

void Shear::loadSettings()
{
    QSettings settings;

    QString lastDir = settings.value("lastDirectory", "C:/").toString();
    int lastDepth = settings.value("folderDepth", 1).toInt();

    ui.linePath->setText(lastDir);
    ui.spinDepth->setValue(lastDepth);
}

void Shear::saveSettings()
{
    QSettings settings;

    settings.setValue("lastDirectory", ui.linePath->text());
    settings.setValue("folderDepth", ui.spinDepth->value());
}

void Shear::closeEvent(QCloseEvent* event)
{
    saveSettings();
    QMainWindow::closeEvent(event);
}