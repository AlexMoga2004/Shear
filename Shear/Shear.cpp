#include "Shear.h"
#include <QFileDialog>  // Needed for the folder picker
#include <QDir>         // Needed to check if a folder exists
#include <QMessageBox>  // Needed for error popups

Shear::Shear(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    // 1. Connect the browse button to our function
    connect(ui.btnBrowse, &QPushButton::clicked, this, &Shear::onBrowseClicked);

    // 2. Connect the text box so it validates when the user hits Enter or clicks away
    connect(ui.linePath, &QLineEdit::editingFinished, this, &Shear::onPathEditingFinished);
}

Shear::~Shear()
{
}

void Shear::onBrowseClicked()
{
    // Open the native Windows folder selection dialog
    QString startDir = ui.linePath->text().isEmpty() ? "C:/" : ui.linePath->text();

    QString dir = QFileDialog::getExistingDirectory(this, "Select Video Directory",
        startDir,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    // If they picked a folder (didn't hit cancel), update the text box
    if (!dir.isEmpty()) {
        ui.linePath->setText(QDir::toNativeSeparators(dir));
    }
}

void Shear::onPathEditingFinished()
{
    QString path = ui.linePath->text();

    // Do nothing if it's empty
    if (path.isEmpty()) return;

    // Check if the directory actually exists on the hard drive
    QDir dir(path);
    if (!dir.exists()) {
        // Pop up an error message
        QMessageBox::warning(this, "Invalid Directory", "The directory you entered does not exist.\nPlease check for typos.");

        // Bring their cursor back to the box and highlight the bad text
        ui.linePath->setFocus();
        ui.linePath->selectAll();
    }
}