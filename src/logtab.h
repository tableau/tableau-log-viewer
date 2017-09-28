#ifndef LOGTAB_H
#define LOGTAB_H

#include "options.h"
#include "statusbar.h"
#include "treemodel.h"
#include "valuedlg.h"

#include <QJsonObject>
#include <QMenu>
#include <QWidget>


namespace Ui {
class LogTab;
}

class LogTab : public QWidget
{
    Q_OBJECT

public:
    explicit LogTab(QWidget *parent, StatusBar *bar, const EventListPtr events);
    ~LogTab();
    bool StartLiveCapture();
    void EndLiveCapture();
    QString GetTabPath() const;
    void SetTabPath(const QString& path);
    void UpdateStatusBar();
    void CopyFullPath();
    void ShowInFolder();

private:
    void keyPressEvent(QKeyEvent *event) override;

    void InitTreeView(const EventListPtr events);
    void InitMenus();
    void InitOneRowMenu();
    void InitTwoRowsMenu();
    void InitMultipleRowsMenu();
    void InitHeaderMenu();
    void SetColumn(int column, int width, bool isHidden);
    void ShowItemDetails(const QModelIndex&);
    void CopyItemDetails(bool textOnly, bool normalized) const;
    void CopyItemDetailsAsHtml() const;
    void CopyItemDetailsAsText() const;
    void CopyItemDetailsAsNormalizedText() const;
    void RowFindPrev();
    void RowFindNext();
    void RowFindImpl(int offset);
    void ShowDetails(const QModelIndex& idx, ValueDlg& valueDlg);
    void ReadFile();
    void ReadDirectoryFiles();
    void SetUpTimer();
    void SetUpFile(std::shared_ptr<QFile> file);
    void UpdateModelView();
    void TrimEventCount();
    bool StartFileLiveCapture();
    void StartDirectoryLiveCapture();
    QString GetDebugInfo() const;

    Ui::LogTab *ui;
    StatusBar *m_bar;
    TreeModel *m_treeModel;
    QMenu *m_oneRowMenu;
    QMenu *m_twoRowsMenu;
    QMenu *m_multipleRowsMenu;
    QMenu *m_headerMenu;
    ValueDlg *m_valueDlg;
    std::vector<std::unique_ptr<QTemporaryFile>> m_tempFiles;
    QAction *m_exportToTabAction;
    QAction *m_highlightSelectedType;
    QAction *m_copyItemsHtmlAction;
    QAction *m_copyItemsTextAction;
    QAction *m_copyItemsNormalizedTextAction;
    QAction *m_showGlobalDateTime;
    QAction *m_showGlobalTime;
    QAction *m_showTimeDeltas;
    int m_openFileMenuIdx;
    int m_eventIndex;
    QFile m_logFile;
    QTimer m_timer;
    std::unique_ptr<QDir> m_liveDirectory;
    QHash<QString, std::shared_ptr<QFile>> m_directoryFiles;
    QList<QString> m_excludedFileNames;
    QString m_tabPath;

private slots:
    void RowDoubleClicked(const QModelIndex& idx);
    void RowRightClicked(const QPoint& pos);
    void RowDiffEvents();
    void RowHideSelected();
    void RowHideSelectedType();
    void RowFindNextSelectedType();
    void RowFindPrevSelectedType();
    void RowHighlightSelectedType();
    void RowShowGlobalDateTime();
    void RowShowGlobalTime();
    void RowShowTimeDeltas();
    void HeaderRightClicked(const QPoint& pos);
    void HeaderItemSelected(QAction* action);
    void ChangeNextIndex();
    void ChangePrevIndex();
    void ExportToNewTab();
    void OpenSelectedFile();

signals:
    void menuUpdateNeeded();
    void exportToTab(QModelIndexList list);
    void openFile(QString);
};

#endif // LOGTAB_H
