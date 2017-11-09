#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "logtab.h"
#include "statusbar.h"
#include "treemodel.h"
#include "ui_mainwindow.h"

#include <memory>
#include <QBitArray>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QModelIndex>
#include <QProgressBar>
#include <QQueue> // Is QQueue a thread safe queue? For that matter, check if we need one for the interactions between client and mainwindow
#include <QTreeView>
#include <QStringList>
#include <QTimer>


class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

public:
    MainWindow(const QStringList& args);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void keyPressEvent(QKeyEvent * k) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void Recent_files_triggered(QAction * action);
    void UpdateTheme();
    void UpdateMenuAndStatusBar();
    void ExportEventsToTab(QModelIndexList list);
    bool LoadLogFile(QString);

    //Slots use underscores as per QT's automatic connection syntax
    //File
    void on_actionOpen_in_new_tab_triggered();
    void on_actionOpen_log_txt_triggered();
    void on_actionOpen_beta_log_txt_triggered();
    void on_actionMerge_into_tab_triggered();
    void on_actionRefresh_triggered();
    void on_actionShow_summary_triggered();
    void on_actionCreate_info_viz_triggered();
    void on_actionSave_filters_triggered();
    void on_menuLoad_filters_aboutToShow();
    void on_menuLoad_filters_triggered(QAction * action);
    void on_actionClose_tab_triggered();
    void on_actionClose_all_tabs_triggered();
    void on_actionExit_triggered();
    //Recent files
    void on_actionClear_Recent_Files_triggered();
    //Highlight
    void on_actionHighlight_triggered();
    void on_actionFind_next_highlighted_triggered();
    void on_actionFind_previous_highlighted_triggered();
    void on_actionHighlight_only_mode_triggered();
    //Find
    void on_actionFind_triggered();
    void on_actionFind_next_triggered();
    void on_actionFind_previous_triggered();

    void on_actionOptions_triggered();
    void on_tabWidget_currentChanged(int index);
    void on_tabWidget_tabCloseRequested(int index);

    void on_actionTail_current_tab_triggered();
    void on_actionClear_all_events_triggered();

    void on_actionLog_directory_triggered();
    void on_actionBeta_log_directory_triggered();
    void on_actionChoose_directory_triggered();

private:
    void WriteSettings();
    void ReadSettings();

    EventListPtr GetEventsFromFile(QString path, int & skippedCount);

    TreeModel * GetCurrentTreeModel();
    QTreeView * GetCurrentTreeView();
    LogTab * GetCurrentLogTab();
    LogTab * GetLogTab(int index);

    void UpdateRecentFilesMenu();
    void ClearRecentFileMenu();
    void AddRecentFile(const QString& path);
    void RemoveRecentFile(const QString& path);

    void MergeLogFile(QString path);
    void FindPrev();
    void FindNext();
    void FindPrevH();
    void FindNextH();
    void FindImpl(int offset, bool findHighlight);

    void StartDirectoryLiveCapture(QString directoryPath, QString label);
    void FocusOpenedFile(QString path);
    LogTab* SetUpTab(EventListPtr events, bool isDirectory, QString path, QString label);

    Options& m_options = Options::GetInstance();
    StatusBar * m_statusBar;
    QStringList m_recentFiles;
    QString m_lastOpenFolder;

    // m_liveFiles is used to store all the files that have been opened/are open in the MainWindow.
    // If a user opens a new tab, this structure is used to check the file path of the file being loaded
    // in a new tab. If the file is open in another tab, the new tab cannot be opened.
    QList<QString> m_allFiles;
};

#endif // MAINWINDOW_H
