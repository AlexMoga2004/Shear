#include "Shear.h"
#include <QFileDialog>  
#include <QDir>        
#include <QMessageBox> 
#include <QSettings>
#include <QApplication>
#include <QProcess>
#include <QFileInfo> 
#include <QCoreApplication>
#include <QDesktopServices>
#include <QUrl>
#include "TrimmerDialog.h"
#include "SettingsDialog.h"

Shear::Shear(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    ui.btnRefresh->setToolTip("Refresh directory");
    ui.btnSettings->setToolTip("Settings");
    ui.btnBrowse->setToolTip("Select folder");
    ui.listThumbnails->setIconSize(QSize(200, 112));

    connect(ui.btnBrowse, &QPushButton::clicked, this, &Shear::onBrowseClicked);
    connect(ui.linePath, &QLineEdit::editingFinished, this, &Shear::onPathEditingFinished);

    connect(ui.btnRefresh, &QPushButton::clicked, this, &Shear::onRefreshClicked);
    connect(ui.btnSettings, &QPushButton::clicked, this, [this]() {
        SettingsDialog dialog(this);
        dialog.exec();
        });
    connect(ui.btnNext, &QPushButton::clicked, this, &Shear::onNextPage);
    connect(ui.btnPrev, &QPushButton::clicked, this, &Shear::onPrevPage);
    connect(ui.spinDepth, &QSpinBox::valueChanged, this, &Shear::onRefreshClicked);

	connect(ui.listThumbnails, &QListWidget::itemDoubleClicked, this, &Shear::onVideoDoubleClicked);

    connect(ui.btnOpenFolder, &QPushButton::clicked, this, [this]() {
        QString currentDir = ui.linePath->text();
        if (!currentDir.isEmpty() && QDir(currentDir).exists()) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(currentDir));
        }
        else {
            QMessageBox::warning(this, "Error", "Directory does not exist.");
        }
        });

    ui.listThumbnails->installEventFilter(this);

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
        QString nativePath = QDir::toNativeSeparators(dir);
        ui.linePath->setText(nativePath);

        scanDirectory(nativePath);
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
    else {
        scanDirectory(path);
    }
}

void Shear::loadSettings()
{
    QSettings settings;

    QString lastDir = settings.value("lastDirectory", "C:/").toString();
    int lastDepth = settings.value("folderDepth", 1).toInt();

    ui.linePath->setText(lastDir);
    ui.spinDepth->setValue(lastDepth);

    scanDirectory(ui.linePath->text());
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

void Shear::onRefreshClicked() {
    scanDirectory(ui.linePath->text());
}

void Shear::scanDirectory(const QString& path)
{
    QDir dir(path);
    if (!dir.exists()) return;

    m_currentDir = path;
    int maxDepth = ui.spinDepth->value();

    m_videoFiles = scanRecursively(dir, maxDepth, 1);

    m_currentPage = 0;
    renderCurrentPage();
}

QStringList Shear::scanRecursively(const QDir& dir, int maxDepth, int currentDepth)
{
    QStringList foundFiles;
    if (currentDepth > maxDepth) return foundFiles;

    QStringList files = dir.entryList({ "*.mp4", "*.mkv", "*.mov", "*.avi" }, QDir::Files | QDir::NoSymLinks, QDir::Time);
    for (const QString& f : files) {
        foundFiles.append(dir.absoluteFilePath(f));
    }

    if (currentDepth < maxDepth) {
        QStringList subdirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
        for (const QString& subdir : subdirs) {
            foundFiles.append(scanRecursively(QDir(dir.absoluteFilePath(subdir)), maxDepth, currentDepth + 1));
        }
    }
    return foundFiles;
}


void Shear::onNextPage() {
    int totalPages = (m_videoFiles.count() + m_itemsPerPage - 1) / m_itemsPerPage;
    if (m_currentPage < totalPages - 1) {
        m_currentPage++;
        renderCurrentPage();
    }
}

void Shear::onPrevPage() {
    if (m_currentPage > 0) {
        m_currentPage--;
        renderCurrentPage();
    }
}

void Shear::renderCurrentPage()
{
    ui.listThumbnails->clear();
    if (m_videoFiles.isEmpty()) {
        ui.labelPage->setText("0 / 0");
        return;
    }

    int totalPages = (m_videoFiles.count() + m_itemsPerPage - 1) / m_itemsPerPage;
    ui.labelPage->setText(QString("Page %1 / %2").arg(m_currentPage + 1).arg(totalPages));

    ui.btnPrev->setEnabled(m_currentPage > 0);
    ui.btnNext->setEnabled(m_currentPage < totalPages - 1);

    int startIndex = m_currentPage * m_itemsPerPage;
    int endIndex = qMin(startIndex + m_itemsPerPage, (int)m_videoFiles.count());

    QPixmap placeholder = getPlaceholder();
    QList<QListWidgetItem*> currentItems;

    for (int i = startIndex; i < endIndex; ++i) {
        QString fullPath = m_videoFiles[i];

        QString displayPath = QDir(m_currentDir).relativeFilePath(fullPath);

        displayPath = QDir::toNativeSeparators(displayPath);

        QListWidgetItem* item = new QListWidgetItem(QIcon(placeholder), displayPath);
        item->setToolTip(fullPath); 
        ui.listThumbnails->addItem(item);
        currentItems.append(item);
    }

    QApplication::processEvents();

    for (int i = 0; i < currentItems.size(); ++i) {
        QString fullPath = m_videoFiles[startIndex + i];
        QPixmap realThumb = generateThumbnail(fullPath);
        currentItems[i]->setIcon(QIcon(realThumb));
        QApplication::processEvents();
    }

    if (ui.listThumbnails->count() > 0) {
        ui.listThumbnails->setCurrentRow(0); 
        ui.listThumbnails->setFocus();       
    }
}

QString Shear::getFFmpegPath()
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString ffmpegPath = QDir(appDir).filePath("bin/ffmpeg/win/ffmpeg.exe");

    if (!QFileInfo::exists(ffmpegPath)) {
        return "ffmpeg";
    }
    return QDir::toNativeSeparators(ffmpegPath);
}

QPixmap Shear::getPlaceholder()
{
    QPixmap pixmap(200, 112);
    pixmap.fill(QColor(255, 0, 0));
    return pixmap;
}

QPixmap Shear::generateThumbnail(const QString& videoPath)
{
    QString tempImagePath = QDir::tempPath() + "/shear_thumb.jpg";
    QProcess ffmpeg;
    QStringList args;

    args << "-y" << "-ss" << "00:00:03" << "-i" << videoPath
        << "-frames:v" << "1" << "-q:v" << "2"
        << "-vf" << "scale=200:-1" << tempImagePath;

    ffmpeg.start(getFFmpegPath(), args);
    ffmpeg.waitForFinished(3000);

    QPixmap pixmap;
    if (ffmpeg.exitCode() == 0) {
        pixmap.load(tempImagePath);
        QFile::remove(tempImagePath);
    }
    else {
        pixmap = getPlaceholder();
    }

    return pixmap;
}

void Shear::onVideoDoubleClicked(QListWidgetItem* item)
{
    QString videoPath = item->toolTip();

    // Spawn our new custom workspace window modally
    TrimmerDialog trimmer(videoPath, m_currentDir, this);
    trimmer.exec();
}

void Shear::keyPressEvent(QKeyEvent* event)
{
    // If the user is typing a folder location, don't intercept the keys
    if (ui.linePath->hasFocus()) {
        QMainWindow::keyPressEvent(event);
        return;
    }

    switch (event->key()) {
    case Qt::Key_Return:
    case Qt::Key_Space:
    case Qt::Key_Enter: {
        if (ui.listThumbnails->hasFocus()) {
            QListWidgetItem* current = ui.listThumbnails->currentItem();
            if (current) {
                onVideoDoubleClicked(current);
            }
        }
        break;
    }

    case Qt::Key_R: {
        onRefreshClicked();
        break;
    }

    case Qt::Key_Comma: { 
        if (ui.btnPrev->isEnabled()) {
            ui.btnPrev->click();
        }
        break;
    }
    case Qt::Key_Period: { 
        if (ui.btnNext->isEnabled()) {
            ui.btnNext->click();
        }
        break;
    }

    default:
        QMainWindow::keyPressEvent(event);
    }
}

bool Shear::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == ui.listThumbnails && event->type() == QEvent::KeyPress) {

        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        int key = keyEvent->key();

        if (key == Qt::Key_R || key == Qt::Key_Comma || key == Qt::Key_Period) {

            keyPressEvent(keyEvent);

            return true;
        }
    }

    return QMainWindow::eventFilter(obj, event);
}