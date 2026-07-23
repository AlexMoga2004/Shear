#pragma once

#include <QtWidgets/QMainWindow>
#include <QCloseEvent>
#include <QStringList>
#include <QPixmap>
#include <QShortcut>
#include <QDir> 
#include "ui_Shear.h"

class Shear : public QMainWindow
{
    Q_OBJECT

public:
    Shear(QWidget* parent = nullptr);
    ~Shear();

protected:
    void closeEvent(QCloseEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    QShortcut* m_shortcutRefresh;
    QShortcut* m_shortcutPrev;
    QShortcut* m_shortcutNext;

private slots:
    void onBrowseClicked();
    void onPathEditingFinished();

    void onRefreshClicked(); 
    void onNextPage();       
    void onPrevPage();       

   void onVideoDoubleClicked(QListWidgetItem* item);


private:
    Ui::ShearClass ui;

    QString m_currentDir;
    QStringList m_videoFiles; 
    int m_currentPage = 0;
    const int m_itemsPerPage = 12;

    void loadSettings();
    void saveSettings();

    void scanDirectory(const QString& path);
    QStringList scanRecursively(const QDir& dir, int maxDepth, int currentDepth); 
    void renderCurrentPage();

    QPixmap getPlaceholder();
    QPixmap generateThumbnail(const QString& videoPath);
    QString getFFmpegPath();
};

