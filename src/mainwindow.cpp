#include "mainwindow.h"

#include "finddlg.h"
#include "highlightdlg.h"
#include "logtab.h"
#include "options.h"
#include "optionsdlg.h"
#include "pathhelper.h"
#include "processevent.h"
#include "savefilterdialog.h"
#include "themeutils.h"
#include "zoomabletreeview.h"

#include <map>

#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QDialogButtonBox>
#include <QDragEnterEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QMimeData>
#include <QScrollBar>
#include <QSettings>
#include <QSignalMapper>
#include <QTime>
#include <QTreeView>

MainWindow::MainWindow()
{
    setupUi(this);

    m_statusBar = new StatusBar(this);

    ReadSettings();

    // Load the theme for the first time
    UpdateTheme();

    UpdateMenuAndStatusBar();

    // About TLV. It will have the Version number, this only needs to be calculated once
    QString dt = __DATE__ " " __TIME__;
    dt.replace("  ", " ");
    QDateTime buildTime = QDateTime::fromString(dt, "MMM d yyyy hh:mm:ss");
    QString timestamp = buildTime.toString("yyyy.MMdd.hhmm");
    QString versionString = QString("Version: %1 (%2)").arg(QApplication::applicationVersion(), timestamp);
    QAction * aboutVersionAction = new QAction(versionString, menuHelp);
    aboutVersionAction->setEnabled(false);
    menuHelp->addAction(aboutVersionAction);

    connect(Ui_MainWindow::menuRecent_files, SIGNAL(triggered(QAction*)), this, SLOT(Recent_files_triggered(QAction*)));

    tabWidget->tabBar()->installEventFilter(this);
}

MainWindow::~MainWindow()
{
}

void MainWindow::Recent_files_triggered(QAction * action)
{
    QString path = action->text();
    QFileInfo fi(path);
    if (fi.isDir())
    {
        StartDirectoryLiveCapture(path, "");
    }
    else if (!LoadLogFile(path))
    {
        RemoveRecentFile(path);
        Ui_MainWindow::menuRecent_files->removeAction(action);
    }
}

QString MainWindow::GetOpenDefaultFolder()
{
    // Prefer the directory of the current tab (if valid)
    LogTab * currentTab = GetCurrentLogTab();
    if (currentTab && currentTab->GetTreeModel()->TabType() == TABTYPE::SingleFile) {
       auto tabPath=currentTab->GetTabPath();
       auto dir=QFileInfo(tabPath).dir();
       if (dir.exists()) {
          return dir.absolutePath();
       }
    }
    // Use the directory used last time (if still valid)
    if (!m_lastOpenFolder.isEmpty() && QDir(m_lastOpenFolder).exists()) {
       return m_lastOpenFolder;
    }
    // Use the Beta log directory
    if (QDir(PathHelper::GetTableauLogFolderPath(true)).exists()) {
       return PathHelper::GetTableauLogFolderPath(true);
    }
    // Use the normal log directory
    if (QDir(PathHelper::GetTableauLogFolderPath(false)).exists()) {
       return PathHelper::GetTableauLogFolderPath(false);
    }
    // Use the documents folder
    if (QDir(PathHelper::GetDocumentsPath()).exists()) {
       return PathHelper::GetDocumentsPath();
    }
    // Last resort: use the current working directory
    return QDir::currentPath();
}

QStringList MainWindow::PickLogFilesToOpen(QString caption)
{
    QFileDialog fileDlg(this, caption, GetOpenDefaultFolder(), "Log Files (*.txt *.log);;All Files (*)");
    fileDlg.setFileMode(QFileDialog::ExistingFiles);
    fileDlg.exec();
    m_lastOpenFolder = fileDlg.directory().absolutePath();
    return fileDlg.selectedFiles();
}

void MainWindow::UpdateRecentFilesMenu()
{
    ClearRecentFileMenu();
    for(const auto & path : m_recentFiles)
    {
        Ui_MainWindow::menuRecent_files->addAction(path);
    }
}

void MainWindow::on_actionClear_Recent_Files_triggered()
{
    m_recentFiles.clear();
    ClearRecentFileMenu();
}

void MainWindow::ClearRecentFileMenu()
{
    Ui_MainWindow::menuRecent_files->clear();
    QAction * clearRecentFiles = new QAction("Clear recent files", Ui_MainWindow::menuRecent_files);
    connect(clearRecentFiles, &QAction::triggered, this, &MainWindow::on_actionClear_Recent_Files_triggered);
    Ui_MainWindow::menuRecent_files->addAction(clearRecentFiles);
    Ui_MainWindow::menuRecent_files->addSeparator();
}

void MainWindow::UpdateTheme()
{
    QString themeName = m_options.getTheme();

    ThemeUtils::SwitchTheme(themeName, this);

    // Update toolbar Icons theme
    this->actionTail_current_tab->setIcon(QIcon(ThemeUtils::GetThemedIcon(":/ctx-livecapture.png")));
    this->actionHighlight_only_mode->setIcon(QIcon(ThemeUtils::GetThemedIcon(":/ctx-highlight.png")));
    this->actionClear_all_events->setIcon(QIcon(ThemeUtils::GetThemedIcon(":/ctx-clear.png")));
    this->actionShow_summary->setIcon(QIcon(ThemeUtils::GetThemedIcon(":/ctx-summary.png")));
    this->actionRefresh->setIcon(QIcon(ThemeUtils::GetThemedIcon(":/ctx-refresh.png")));
    this->actionFind->setIcon(QIcon(ThemeUtils::GetThemedIcon(":/ctx-find.png")));
    this->actionOpen_in_new_tab->setIcon(QIcon(ThemeUtils::GetThemedIcon(":/ctx-open.png")));
    this->actionMerge_into_tab->setIcon(QIcon(ThemeUtils::GetThemedIcon(":/ctx-open-merge.png")));
}

void MainWindow::UpdateMenuAndStatusBar()
{
    LogTab * logTab = GetCurrentLogTab();
    TreeModel * model = logTab ? logTab->GetTreeModel() : nullptr;
    bool hasFilters = model && model->HasHighlightFilters();
    bool hasFindOpts = model && model->ValidFindOpts();

    // Menu items
    menuHighlight->setEnabled(logTab);
    menuFind->setEnabled(logTab);
    // QActions
    // File
    actionMerge_into_tab->setEnabled(logTab);
    actionClear_all_events->setEnabled(logTab);
    actionRefresh->setEnabled(logTab);
    actionShow_summary->setEnabled(logTab);
    actionCreate_info_viz->setEnabled(logTab);
    actionClose_tab->setEnabled(logTab);
    actionClose_all_tabs->setEnabled(logTab);
    //Recent Files
    UpdateRecentFilesMenu();
    //Highlight
    actionFind_next_highlighted->setEnabled(hasFilters);
    actionFind_previous_highlighted->setEnabled(hasFilters);
    actionHighlight_only_mode->setEnabled(hasFilters);
    actionHighlight_only_mode->setChecked(model && model->m_highlightOnlyMode);
    menuLoad_filters->setEnabled(logTab);
    actionSave_filters->setEnabled(hasFilters);
    //Find
    actionFind->setEnabled(model);
    actionFind_next->setEnabled(hasFindOpts);
    actionFind_previous->setEnabled(hasFindOpts);
    //Live capture
    actionTail_current_tab->setEnabled(model && model->TabType() != TABTYPE::ExportedEvents);
    actionTail_current_tab->setChecked(model && model->m_liveMode);

    // Status bar
    if (logTab == nullptr)
    {
        m_statusBar->SetRightLabelText("¯\\_(ツ)_/¯");
        return;
    }

    if (logTab)
    {
        logTab->UpdateStatusBar();
    }
}

void MainWindow::WriteSettings()
{
    QString iniPath = PathHelper::GetConfigIniPath();
    QSettings settings(iniPath, QSettings::IniFormat);

    settings.beginGroup("MainWindow");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.setValue("recentFiles", m_recentFiles);
    settings.setValue("lastOpenFolder", m_lastOpenFolder);
    settings.endGroup();

    ValueDlg::WriteSettings(settings);
    ZoomableTreeView::WriteSettings(settings);
}

void MainWindow::ReadSettings()
{
    QString iniPath = PathHelper::GetConfigIniPath();
    QSettings settings(iniPath, QSettings::IniFormat);

    settings.beginGroup("MainWindow");
    if (settings.value("size").isNull())
    {
        restoreGeometry(settings.value("geometry").toByteArray());
        restoreState(settings.value("windowState").toByteArray());
    }
    else
    {
        // Backcompat: size and pos are now replaced with geometry that works with windowState.
        resize(settings.value("size").toSize());
        move(settings.value("pos").toPoint());
        settings.remove("size");
        settings.remove("pos");
    }
    m_recentFiles = settings.value("recentFiles", QStringList()).toStringList();
    m_lastOpenFolder = settings.value("lastOpenFolder", QString()).toString();
    settings.endGroup();

    //Load Options variables from config file
    m_options.ReadSettings();

    ValueDlg::ReadSettings(settings);
    ZoomableTreeView::ReadSettings(settings);
}

EventListPtr MainWindow::GetEventsFromFile(QString path, int & skippedCount)
{
    auto events = std::make_shared<EventList>();
    QFile logfile(path);
    QFileInfo logfileinfo(path);

    int eventCount = 0;

    if (logfile.open(QIODevice::ReadOnly))
    {
        while (!logfile.atEnd())
        {
            auto line = logfile.readLine().trimmed();
            if (line.isEmpty())
            {
                continue;
            }
            QJsonObject ev = ProcessEvent::ProcessLogEventMessage(++eventCount, line, logfileinfo.fileName());
            if (!ev.isEmpty())
            {
                events->append(ev);
            }
            else
            {
                skippedCount++;
            }
        }
        logfile.close();
    }
    return events;
}

void MainWindow::ExportEventsToTab(QModelIndexList list, QString name)
{
    auto events = std::make_shared<EventList>();
    TreeModel * model = GetCurrentTreeModel();
    QTreeView * view = GetCurrentTreeView();
    for (QModelIndex event : list)
    {
        if (event.parent().row() == -1 || !list.contains(event.parent()))
        {
            QJsonObject jsonData = model->GetEvent(event);
            events->append(jsonData);
        }
    }

    LogTab * logTab = new LogTab(tabWidget, m_statusBar, events);
    QTreeView * exportedView = logTab->GetTreeView();
    TreeModel * exportedModel = logTab->GetTreeModel();

    exportedModel->SetTabType(TABTYPE::ExportedEvents);
    // Inherit highlight filters
    exportedModel->SetHighlightFilters(model->GetHighlightFilters());
    // Inherit the column layout
    for (int column=0; column<exportedModel->columnCount(); ++column) {
       exportedView->setColumnWidth(column, view->columnWidth(column));
       exportedView->setColumnHidden(column, view->isColumnHidden(column));
    }
    // Expand same items in exported view as in original view
    int exportCount = 0;
    for (QModelIndex event : list)
    {
        if (event.parent().row() == -1)
        {
            QModelIndex exportIndex = exportedModel->index(exportCount, 0);
            if (view->isExpanded(event))
            {
                exportedView->expand(exportIndex);
            }
        }
    }

    actionTail_current_tab->setEnabled(false);
    connect(logTab, &LogTab::menuUpdateNeeded, this, &MainWindow::UpdateMenuAndStatusBar);
    connect(logTab, &LogTab::exportToTab, this, &MainWindow::ExportEventsToTab);
    QString shortName = name.size() < 30 ? name : (name.left(27) + "...");
    int idx = tabWidget->addTab(logTab, shortName);
    tabWidget->setTabToolTip(idx, name);
    tabWidget->setCurrentIndex(idx);
    logTab->setFocus();
}

void MainWindow::AddRecentFile(const QString& path)
{
    int indexOfPath = m_recentFiles.indexOf(path);
    //If file is in recent, move it to the top
    if(indexOfPath != -1)
    {
        m_recentFiles.removeAt(indexOfPath);
        m_recentFiles.prepend(path);
    }
    //Otherwise add it to the top of the list
    else if(path != "")
    {
        if (m_recentFiles.size() > 14)
        {
            m_recentFiles.removeLast();
        }
        m_recentFiles.prepend(path);
    }
}

void MainWindow::RemoveRecentFile(const QString& path)
{
    int indexOfPath = m_recentFiles.indexOf(path);
    if (indexOfPath >= 0)
    {
        m_recentFiles.removeAt(indexOfPath);
    }
}

void MainWindow::MergeLogFile(QString path)
{
    EventListPtr events;
    QString fileName;
    QString filePath;
    int skippedCount = 0;

    QFileInfo fi(path);
    if (!fi.exists() || !fi.isFile())
    {
        QMessageBox::warning(this, tr("Unable to open file"), tr("Unable to open file \"%1\"").arg(path));
        return;
    }
    events = GetEventsFromFile(path, skippedCount);

    fileName = fi.fileName();
    filePath = fi.filePath();

    // Set tab text
    int currIdx = tabWidget->currentIndex();
    tabWidget->setTabText(currIdx, tabWidget->tabText(currIdx) + ", " + fileName);

    // Merge events in, add file name to model's paths
    TreeModel * model = GetCurrentTreeModel();
    model->MergeIntoModelData(*events);
    model->m_paths.append(filePath);

    // Update status
    LogTab * logTab = GetCurrentLogTab();
    statusBar()->showMessage(QString("%1 events loaded; %2 events skipped").arg(QString::number(logTab->GetTreeModel()->rowCount()), QString::number(skippedCount)), 3000);
    UpdateMenuAndStatusBar();
}

void MainWindow::on_actionTail_current_tab_triggered()
{
    LogTab * currentTab = GetCurrentLogTab();

    if (!currentTab)
        return;

    if (actionTail_current_tab->isChecked())
    {
        if (currentTab->StartLiveCapture())
        {
            currentTab->GetTreeView()->scrollToBottom();
            tabWidget->setTabIcon(tabWidget->currentIndex(), QIcon(ThemeUtils::GetThemedIcon(":/tab-sync-thin.png")));
        }
        else
        {
            actionTail_current_tab->setChecked(false);
        }
    }
    else
    {
        tabWidget->setTabIcon(tabWidget->currentIndex(), QIcon());
        currentTab->EndLiveCapture();
    }
}

void MainWindow::on_actionClear_all_events_triggered()
{
    TreeModel * model = GetCurrentTreeModel();
    if (model != nullptr)
    {
        model->removeRows(0, model->rowCount());
    }
}

inline QString SystemCase(const QString& path)
{
#ifdef Q_OS_WIN32
    return path.toLower();
#else
    return path;
#endif
}

bool MainWindow::LoadLogFile(QString path)
{
    EventListPtr events;
    QString fileName;
    QString filePath;
    int skippedCount = 0;

    QFileInfo fi(path);
    if (!fi.exists() || !fi.isFile())
    {
        QMessageBox::warning(this, tr("Unable to open file"), tr("Unable to open file \"%1\"").arg(path));
        return false;
    }
    events = GetEventsFromFile(path, skippedCount);

    fileName = fi.fileName();
    filePath = fi.filePath();
	path.replace("\\", "/");
    if (!m_allFiles.contains(SystemCase(filePath)))
    {
        SetUpTab(events, false, path, fileName);
    }
    else
    {
        FocusOpenedFile(path);
    }
    return true;
}

void MainWindow::StartDirectoryLiveCapture(QString directoryPath, QString label)
{
    directoryPath.replace("\\", "/");
    if (!m_allFiles.contains(SystemCase(directoryPath)))
    {
        if (label.isEmpty())
        {
            QFileInfo fi(directoryPath);
            label = QString("%1 directory").arg(fi.fileName());
        }
        EventListPtr events = std::make_shared<EventList>();
        SetUpTab(events, true, directoryPath, label);
    }
    else
    {
        FocusOpenedFile(directoryPath);
    }
}

LogTab* MainWindow::SetUpTab(EventListPtr events, bool isDirectory, QString path, QString label)
{
    LogTab * logTab = new LogTab(tabWidget, m_statusBar, events);
    connect(logTab, &LogTab::menuUpdateNeeded, this, &MainWindow::UpdateMenuAndStatusBar);
    connect(logTab, &LogTab::exportToTab, this, &MainWindow::ExportEventsToTab);
    connect(logTab, &LogTab::openFile, this, &MainWindow::LoadLogFile);
    int idx = tabWidget->addTab(logTab, label);
    m_allFiles.append(SystemCase(path));

    TreeModel* model = logTab->GetTreeModel();

    if (!isDirectory)
    {
        model->m_paths.append(path);
        model->SetTabType(TABTYPE::SingleFile);
    }
    else
    {
        model->SetTabType(TABTYPE::Directory);
    }
    AddRecentFile(path);
    logTab->SetTabPath(path);
    actionTail_current_tab->setEnabled(model->TabType() != TABTYPE::ExportedEvents);

    tabWidget->setTabToolTip(idx, path);
    tabWidget->setCurrentIndex(idx);
    logTab->setFocus();

    bool futureTabsUnderLive = m_options.getFutureTabsUnderLive();
    if (isDirectory || futureTabsUnderLive)
    {
        actionTail_current_tab->setChecked(true);
        on_actionTail_current_tab_triggered();
    }
    UpdateMenuAndStatusBar();
    return logTab;
}

void MainWindow::FocusOpenedFile(QString path)
{
    path.replace("\\", "/");
    for (int i = 0; i < tabWidget->count(); i++)
    {
        LogTab * currentTab = GetLogTab(i);
        if (currentTab && currentTab->GetTabPath().compare(path) == 0)
        {
            tabWidget->setCurrentIndex(i);
            currentTab->setFocus();
            break;
        }
    }
}

void MainWindow::on_actionLog_directory_triggered()
{
    QString directoryPath = PathHelper::GetTableauLogFolderPath(false);
    StartDirectoryLiveCapture(directoryPath, "Log directory");
}

void MainWindow::on_actionBeta_log_directory_triggered()
{
    QString directoryPath = PathHelper::GetTableauLogFolderPath(true);
    StartDirectoryLiveCapture(directoryPath, "Beta log directory");
}

void MainWindow::on_actionChoose_directory_triggered()
{
    QString directoryPath = QFileDialog::getExistingDirectory(this, "Select directory to monitor", GetOpenDefaultFolder());
    if (!directoryPath.isEmpty())
    {
        StartDirectoryLiveCapture(directoryPath, "");
        m_lastOpenFolder = directoryPath;
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls())
    {
        for (const QUrl &url : e->mimeData()->urls())
        {
            QFileInfo fi(QString(url.toLocalFile()));
            if (fi.suffix().compare("txt", Qt::CaseInsensitive) != 0 &&
                fi.suffix().compare("log", Qt::CaseInsensitive) != 0 &&
                !fi.isDir())
                return;
        }
        e->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *e)
{
    e->acceptProposedAction();
    foreach (const QUrl &url, e->mimeData()->urls())
    {
        const QString &fileName = url.toLocalFile();
        QFileInfo fi(fileName);
        if (fi.isDir())
        {
            StartDirectoryLiveCapture(fileName, "");
        }
        else
        {
            LoadLogFile(fileName);
        }
    }
}

void MainWindow::keyPressEvent(QKeyEvent * k)
{
    if ((k->key() == Qt::Key_W) && (QApplication::keyboardModifiers() & Qt::ControlModifier))
    {
        on_actionClose_tab_triggered();
    }
    else
    {
        QMainWindow::keyPressEvent(k);
    }
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    if (index == -1)
        return;
    UpdateMenuAndStatusBar();
}

void MainWindow::on_tabWidget_tabCloseRequested(int index)
{
    if (index == -1)
        return;

    LogTab * logTab = GetLogTab(index);
    m_allFiles.removeAll(SystemCase(logTab->GetTabPath()));
    logTab->EndLiveCapture();

    QWidget* tabItem = tabWidget->widget(index);
    tabWidget->removeTab(index);
    delete tabItem;
    UpdateMenuAndStatusBar();
}

void MainWindow::on_actionOpen_in_new_tab_triggered()
{
    QStringList files = PickLogFilesToOpen("Select one or more log files to open");
    foreach(auto file, files)
        LoadLogFile(file);
}

void MainWindow::on_actionOpen_log_txt_triggered()
{
    if (!LoadLogFile(PathHelper::GetTableauLogFilePath(false)))
    {
        return;
    }

    if (!actionTail_current_tab->isChecked())
    {
        actionTail_current_tab->setChecked(true);
        on_actionTail_current_tab_triggered();
    }
}

void MainWindow::on_actionOpen_beta_log_txt_triggered()
{
    if (!LoadLogFile(PathHelper::GetTableauLogFilePath(true)))
    {
        return;
    }

    if (!actionTail_current_tab->isChecked())
    {
        actionTail_current_tab->setChecked(true);
        on_actionTail_current_tab_triggered();
    }
}

void MainWindow::on_actionMerge_into_tab_triggered()
{
    QStringList files = PickLogFilesToOpen("Select one or more log files to merge into current tab");
    TreeModel * model = GetCurrentTreeModel();
    bool merged = false;
    foreach(auto file, files)
    {
        if(!model->m_paths.contains(file))
        {
            merged = true;
            MergeLogFile(file);
        }
    }
    if(merged)
    {
        QTreeView * treeView = GetCurrentTreeView();
        treeView->setColumnHidden(COL::File, false);
    }

    UpdateMenuAndStatusBar();
}

void MainWindow::on_actionRefresh_triggered()
{
    TreeModel * model = GetCurrentTreeModel();
    if (!model)
        return;

    // TODO: Save the current line ID and go back to the line after refresh.

    int skipped = 0;
    model->removeRows(0, model->rowCount());
    for (QString path : model->m_paths)
    {
        EventListPtr events(GetEventsFromFile(path, skipped));
        model->MergeIntoModelData(*events);
    }

    if (model->m_highlightOnlyMode)
    {
        GetCurrentLogTab()->RefilterTreeView();
    }

    UpdateMenuAndStatusBar();
}

void MainWindow::on_actionSave_filters_triggered()
{
    TreeModel * model = GetCurrentTreeModel();
    if (model && model->HasHighlightFilters())
    {
        SaveFilterDialog saveFilterDialog(this, model->GetHighlightFilters());
        saveFilterDialog.exec();
    }
}

void MainWindow::on_menuLoad_filters_aboutToShow()
{
    QMenu* loadMenu = Ui_MainWindow::menuLoad_filters;
    loadMenu->clear();

    QDir loadDir(PathHelper::GetFiltersConfigPath());
    bool filtersAvailable = false;
    if (loadDir.exists())
    {
        QString filterExtension = QStringLiteral("*.json");
        for (QString entry : loadDir.entryList(QStringList(filterExtension), QDir::Filter::Files))
        {
            // Add an action with the filter file. Take out the ".json" at the end
            loadMenu->addAction(entry.left(entry.length() - filterExtension.length() + 1));
            filtersAvailable = true;
        }
    }
    if (!filtersAvailable)
    {
        QAction *noFiltersAction = loadMenu->addAction("(no filters available)");
        noFiltersAction->setEnabled(false);
    }
}

void MainWindow::on_menuLoad_filters_triggered(QAction * action)
{
    QString file(action->text() + ".json");
    TreeModel * model = GetCurrentTreeModel();
    if (model == nullptr)
    {
        return;
    }
    QDir loadDir(PathHelper::GetFiltersConfigPath());
    QFile loadFile(loadDir.filePath(file));
    if (!loadFile.open(QIODevice::ReadOnly))
    {
        qWarning("Couldn't open file.");
        return;
    }
    QByteArray filterData = loadFile.readAll();
    QJsonDocument filtersDoc(QJsonDocument::fromJson(filterData));
    model->SetHighlightFilters(HighlightOptions(filtersDoc.array()));

    GetCurrentLogTab()->RefilterTreeView();
    UpdateMenuAndStatusBar();
}

void MainWindow::on_actionClose_tab_triggered()
{
    on_tabWidget_tabCloseRequested(tabWidget->currentIndex());
}

void MainWindow::on_actionClose_all_tabs_triggered()
{
    for (int i = 0; i < tabWidget->count(); i++)
    {
        LogTab * logTab = GetLogTab(i);
        logTab->EndLiveCapture();
    }
    tabWidget->clear();
    m_allFiles.clear();
    UpdateMenuAndStatusBar();
}

void MainWindow::on_actionExit_triggered()
{
    close();
}

void MainWindow::closeEvent(QCloseEvent * event)
{
    event->ignore();
    WriteSettings();
    event->accept();
}

//Highlight
void MainWindow::on_actionHighlight_triggered()
{
    auto model = GetCurrentTreeModel();
    if (model == nullptr)
        return;

    // Construct a highlight dialog with our model's current _HighlightOpts.
    HighlightDlg highlightDlg(this, model->GetHighlightFilters(), model->m_colorLibrary);

    // Open the dialog and see if ok was pressed. If so we want to update our model's _HighlightOpts member.
    if (highlightDlg.exec() == QDialog::Accepted)
    {
        model->SetHighlightFilters(highlightDlg.m_highlightOpts);
        model->m_colorLibrary = highlightDlg.m_colorLibrary;

        if (!model->HasHighlightFilters())
        {
            model->m_highlightOnlyMode = false;
        }

        GetCurrentLogTab()->RefilterTreeView();
        UpdateMenuAndStatusBar();
    }
}

void MainWindow::on_actionFind_next_highlighted_triggered()
{
    FindNextH();
}
void MainWindow::on_actionFind_previous_highlighted_triggered()
{
    FindPrevH();
}

void MainWindow::on_actionHighlight_only_mode_triggered()
{
    auto model = GetCurrentTreeModel();
    if (model == nullptr)
        return;

    //Don't turn highlight only mode on if there are no filters. But DO turn highlight only mode off if there are no filters
    if (!model->m_highlightOnlyMode && !model->HasHighlightFilters())
        return;
    model->m_highlightOnlyMode = !model->m_highlightOnlyMode;

    GetCurrentLogTab()->RefilterTreeView();
    UpdateMenuAndStatusBar();
}

//Find
void MainWindow::on_actionFind_triggered()
{
    TreeModel * model = GetCurrentTreeModel();
    if (model == nullptr)
        return;

    // Construct a highlight dialog with our model's current _HighlightOpts.
    FindDlg findDlg(this, model->m_findOpts);
    // Connect the Find next/previous functions of the dialog to the find functionality in the main window
    connect(&findDlg, &FindDlg::next,this, [model, &findDlg, this]() {
        model->m_findOpts = findDlg.m_findOpts;
        UpdateMenuAndStatusBar();
        FindNext();
    });
    connect(&findDlg, &FindDlg::prev, this, [model, &findDlg, this]() {
        model->m_findOpts = findDlg.m_findOpts;
        UpdateMenuAndStatusBar();
        FindPrev();
    });

    // Open the dialog and see if ok was pressed
    if (findDlg.exec() == QDialog::Accepted)
    {
        model->m_findOpts = findDlg.m_findOpts;
        UpdateMenuAndStatusBar();
        FindNext();
    }
}

void MainWindow::on_actionFind_next_triggered()
{
    FindNext();
}

void MainWindow::on_actionFind_previous_triggered()
{
    FindPrev();
}

void MainWindow::on_actionOptions_triggered()
{
    QString prevThemeName = m_options.getTheme();

    OptionsDlg optionsDlg(this);
    optionsDlg.exec();

    //Update if user change theme
    if (prevThemeName != m_options.getTheme())
        UpdateTheme();
}

QString msecsToString(qint64 mseconds)
{
    const qint64 msPerDay = 24 * 60 * 60 * 1000;
    qint64 days = mseconds / msPerDay;
    QTime t = QTime(0, 0).addMSecs(mseconds % msPerDay);
    QString str = (days > 0) ? QString("%1 day(s), ").arg(days) : "";
    str += QString("%1:%2:%3.%4 h:m:s")
            .arg(t.hour(), 2, 10, QChar('0'))
            .arg(t.minute(), 2, 10, QChar('0'))
            .arg(t.second(), 2, 10, QChar('0'))
            .arg(t.msec(), 2, 10, QChar('0'));
    return str;
}

void ShowSummary(TreeModel* model, QWidget* parent)
{
    typedef std::map<QString, int> CounterMap;

    struct SummaryCounter
    {
        QString Description;
        QString Key;
        QString Value;
        int Count;
        std::unique_ptr<CounterMap> SubCounters;
    };

    QString beginQueryEventKey = "begin-query";
    for (int i = 0; i < model->rowCount(); i++)
    {
        QModelIndex valIndex = model->index(i, COL::Value);
        QString keyString = model->GetEvent(valIndex)["k"].toString();
        if (keyString == "begin-query")
        {
            break;
        }
        else if (keyString == "begin-protocol.query")
        {
            beginQueryEventKey = "begin-protocol.query";
            break;
        }
    }

    SummaryCounter counters[] {
        { "Workbook opened", "command-post", "tabui:open-workbook", 0, nullptr },
        { "Query batch", "qp-batch-summary", nullptr, 0, nullptr },
        { "Query", beginQueryEventKey, nullptr, 0, nullptr },
        { "Query category", beginQueryEventKey, "query-category", 0, std::make_unique<CounterMap>() },
    };

    const int rowCount = model->rowCount();
    for (int i = 0; i < rowCount; i++)
    {
        QModelIndex valIndex = model->index(i, COL::Value);
        QString valString = model->GetValueFullString(valIndex);
        QJsonObject event = model->GetEvent(valIndex);
        QString keyString = event["k"].toString();
        auto valObj = event["v"];
        for (SummaryCounter& counter : counters)
        {
            // Must match full key string.
            if (keyString != counter.Key)
                continue;

            // Simple key only or key-value counter.
            if (counter.Value.isNull() ||
                (valString.contains(counter.Value)))
            {
                counter.Count++;
            }

            // Sub-counters: match counter.Value with the key part of "v" key-value pairs,
            // and bucket-count the different values.
            if (counter.SubCounters && valObj.isObject())
            {
                QJsonObject json = valObj.toObject();
                QString& subKey = counter.Value;
                if (json.contains(subKey))
                {
                    QString subValue = json[subKey].toString();
                    (*counter.SubCounters)[subValue]++;
                }
            }
        }
    }

    QString summaryText;
    if (rowCount > 0)
    {
        auto firstIdx = model->index(0, COL::Time);
        auto lastIdx = model->index(model->rowCount() - 1, COL::Time);
        QString firstTimestamp = model->data(firstIdx, Qt::DisplayRole).toString();
        QString lastTimestamp = model->data(lastIdx, Qt::DisplayRole).toString();
        auto firstDT = model->data(firstIdx, Qt::UserRole).toLongLong();
        auto lastDT = model->data(lastIdx, Qt::UserRole).toLongLong();
        auto diff = (lastDT - firstDT);
        summaryText += QString("Begin: %1\nEnd: %2\nSpan: %3\n\n")
                .arg(firstTimestamp).arg(lastTimestamp).arg(msecsToString(diff));
    }

    summaryText += "Number of";
    summaryText += QString("\n    Event: %L1").arg(rowCount);
    for (const SummaryCounter& counter : counters)
    {
        summaryText += QString("\n    %1: %L2").arg(counter.Description).arg(counter.Count);
        if (counter.SubCounters && counter.SubCounters->size())
        {
            for (const auto& sc : *counter.SubCounters)
            {
                int subCount = sc.second;
                float subCountPercent = subCount * 100.0 / counter.Count;
                summaryText += QString("\n    - %1: %L2 (%3%)").arg(sc.first).arg(subCount, 3).arg(subCountPercent, 0, 'f', 1);
            }
        }
    }

    QMessageBox msgBox(parent);
    msgBox.setWindowTitle("Summary");
    msgBox.setText(summaryText);
    msgBox.exec();
}

void MainWindow::on_actionShow_summary_triggered()
{
    ShowSummary(GetCurrentTreeModel(), this);
}

void ConvertJsonToStringMap(const QJsonObject& valJson, const QStringList& fields, QMap<QString, QString>& nameValues)
{
    for (const auto& field : fields)
    {
        const auto& val = valJson[field];
        if (val.isDouble())
        {
            double intpart;
            const int decimals = (modf(val.toDouble(), &intpart) == 0) ? 0 : 3;
            nameValues[field] = QString::number(val.toDouble(), 'f', decimals);
        }
        else if (val.isString())
        {
            QString strVal = val.toString();
            strVal.truncate(32000); // Excel cannot handle more than ~32K chars.
            nameValues[field] = "\"" % strVal.replace("\n", "\\n").replace("\"", "\"\"") % "\"";
        }
        else if (val.isObject())
        {
            ConvertJsonToStringMap(val.toObject(), fields, nameValues);
        }
        else if (!nameValues.contains(field))
        {
            nameValues[field] = "";
        }
    }
}

void WriteJsonAsCsv(const QJsonObject& valJson, QStringList& fields, QString& outputStr)
{
    // If list of fields is not specified, set it to all fields available in the first event.
    if (fields.isEmpty())
    {
        fields = valJson.keys();
    }

    QMap<QString, QString> nameValues;

    ConvertJsonToStringMap(valJson, fields, nameValues);

    for (const auto& field : fields)
    {
        outputStr += nameValues[field] + ",";
    }
    outputStr.truncate(outputStr.size() - 1);
    outputStr += "\n";
}

bool CreateFolder(const QString& path)
{
    QDir folder(path);
    if (!folder.exists() && !folder.mkpath("."))
    {
        QMessageBox warning(
            QMessageBox::Icon::Warning,
            "Error creating directory",
            folder.path());
        warning.exec();
        return false;
    }
    return true;
}

bool SaveFile(const QString& path, const QString& filename, const QStringList& headers, const QString& content)
{
    QFile file(path + "/" + filename);
    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox warning(
            QMessageBox::Icon::Warning,
            "Error writing file",
            file.fileName());
        warning.exec();
        return false;
    }

    QString output = headers.join(',') + "\n" + content;
    file.write(output.toUtf8().data());
    file.close();
    return true;
}

bool CopyAllFiles(const QString& fromPath, const QString& toPath)
{
    QDir fromFolder(fromPath);
    if (!fromFolder.exists())
    {
        QMessageBox warning(QMessageBox::Icon::Warning, "Directory not found", fromFolder.path());
        warning.exec();
        return false;
    }

    QDir toFolder(toPath);
    if (!toFolder.exists())
    {
        QMessageBox warning(QMessageBox::Icon::Warning, "Directory not found", toFolder.path());
        warning.exec();
        return false;
    }

    QStringList files = fromFolder.entryList(QDir::Files);
    for (const QString& fileName : files)
    {
        QFileInfo fromFile{fromPath + "/" + fileName};
        QFileInfo toFile{toPath + "/" + fileName};

        if (toFile.exists())
        {
            if (fromFile.lastModified() > toFile.lastModified())
            {
                QFile::remove(toFile.filePath());
            }
            else
            {
                // Skip copying if destination is the same version or newer.
                continue;
            }
        }

        if (!QFile::copy(fromFile.filePath(), toFile.filePath()))
        {
            QMessageBox warning(
                QMessageBox::Icon::Warning, "Cannot copy file",
                "From " + fromFile.filePath() + "\nTo " + toFile.filePath());
            warning.exec();
            return false;
        }
    }

    return true;
}

void GeQueryInfoViz(TreeModel* model)
{
    if (!model)
        return;

    QString outputQuery;
    QString outputTempTable;
    QString outputProtocol;
    QString outputFedTempTable;
    QString outputElapsedEvent;
    QStringList fieldsQuery{"cols", "elapsed", "protocol-id", "query", "query-category", "query-hash", "rows"};
    QStringList fieldsTempTable{"elapsed",    "elapsed-create", "elapsed-insert",    "num-columns",
                                "num-tuples", "protocol-id",    "source-query-hash", "tablename"};
    QStringList fieldsProtocol{"id", "created-elapsed", "attributes", "class", "dbname", "server"};
    QStringList fieldsFedTempTable{"query-hash", "table-name"};
    QStringList fieldsElapsedEvent{"line-id", "time", "elapsed", "begin", "file", "pid", "event", "value"};

    const int rowCount = model->rowCount();
    for (int i = 0; i < rowCount; i++)
    {
        QModelIndex valIndex = model->index(i, COL::Value);
        QJsonObject event = model->GetEvent(valIndex);
        QString keyString = event["k"].toString();

        auto elapsed = model->index(i, COL::Elapsed).data().toDouble();
        if (elapsed != 0.0)
        {
            auto strId = model->index(i, COL::ID).data().toString();
            auto strTime = event["ts"].toString();
            auto strElapsed = QString::number(elapsed, 'f', 3);
            auto beginTime = model->index(i, COL::Time).data(Qt::UserRole).toDateTime().toMSecsSinceEpoch() - (elapsed * 1000);
            auto strFile = model->index(i, COL::File).data().toString();
            auto strPid = model->index(i, COL::PID).data().toString();
            auto strValue = model->GetValueFullString(valIndex);
            if (strValue.size() > 3000)
            {
                strValue.truncate(3000);
                strValue += "...";
            }
            strValue.replace("\n", "\\n").replace("\"", "\"\"");
            outputElapsedEvent += QString("%1,\"%2\",%3,%4,\"%5\",%6,\"%7\",\"%8\"\n")
                                      .arg(
                                          strId, strTime, strElapsed, QString::number(beginTime, 'f', 0), strFile,
                                          strPid, keyString, strValue);
        }

        if (keyString == "end-query")
        {
            const QJsonObject& valJson = event["v"].toObject();
            WriteJsonAsCsv(valJson, fieldsQuery, outputQuery);

            // For federated queries, parsed out all the temp tables used and create a separate list.
            QString queryText = valJson["query"].toString();
            if (queryText.contains("FQ_Temp_"))
            {
                QRegularExpression regex;
                if (queryText.startsWith("(restrict"))
                {
                    regex.setPattern("table (.*?)\\)");
                }
                else if (queryText.startsWith("SELECT "))
                {
                    regex.setPattern("\"(#Tableau_.*?)\" ");
                }
                else
                {
                    continue;
                }

                QString queryHash = QString::number(valJson["query-hash"].toDouble(), 'f', 0);
                QRegularExpressionMatchIterator i = regex.globalMatch(queryText);
                while (i.hasNext())
                {
                    QRegularExpressionMatch match = i.next();
                    QString tableName = match.captured(1);
                    if (!tableName.startsWith("["))
                    {
                        // Hyper does not use square brackets for table names, but the table names
                        // we log in sql-temp-table events have them.
                        tableName = "[" % tableName % "]";
                    }
                    outputFedTempTable += queryHash % ",\"" % tableName % "\"\n";
                }
            }
        }
        else if (keyString == "end-sql-temp-table-tuples-create")
        {
            WriteJsonAsCsv(event["v"].toObject(), fieldsTempTable, outputTempTable);
        }
        else if (keyString == "construct-protocol")
        {
            WriteJsonAsCsv(event["v"].toObject(), fieldsProtocol, outputProtocol);
        }
    }

    if (outputElapsedEvent.isEmpty())
    {
        QMessageBox warning(QMessageBox::Icon::Warning, "Error", "No parsable event found.");
        warning.exec();
        return;
    }

    QString docFolderPath = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation)[0] + "/TLV";
    if (CreateFolder(docFolderPath) &&
        SaveFile(docFolderPath, "TLV_Query.csv", fieldsQuery, outputQuery) &&
        SaveFile(docFolderPath, "TLV_FedTempTable.csv", fieldsFedTempTable, outputFedTempTable) &&
        SaveFile(docFolderPath, "TLV_TempTable.csv", fieldsTempTable, outputTempTable) &&
        SaveFile(docFolderPath, "TLV_Protocol.csv", fieldsProtocol, outputProtocol) &&
        SaveFile(docFolderPath, "TLV_ElapsedEvent.csv", fieldsElapsedEvent, outputElapsedEvent))
    {
        CopyAllFiles(":/workbooks", docFolderPath);
        QDesktopServices::openUrl(QUrl::fromLocalFile(docFolderPath));
    }
}

void MainWindow::on_actionCreate_info_viz_triggered()
{
    GeQueryInfoViz(GetCurrentTreeModel());
}

void MainWindow::FindPrev()
{
    FindImpl(-1, false);
}
void MainWindow::FindNext()
{
    FindImpl(1, false);
}

void MainWindow::FindPrevH()
{
    FindImpl(-1, true);
}

void MainWindow::FindNextH()
{
    FindImpl(1, true);
}

void MainWindow::FindImpl(int offset, bool findHighlight)
{
    QTreeView * tree = GetCurrentTreeView();
    TreeModel * model = GetCurrentTreeModel();

    if (model->rowCount() == 0)
        return;

    QVector<SearchOpt> findFilter{model->m_findOpts};
    const QVector<SearchOpt>& filters = (findHighlight) ?
        static_cast<const QVector<SearchOpt>&>(model->GetHighlightFilters()) :
        findFilter;

    int start = tree->currentIndex().row();
    // If nothing is selected, the current index is -1. Force to start at 0 to avoid an infinite loop.
    if (start < 0)
    {
        start = 0;
    }
    int i = start;

    // Used for isRowHidden()
    QModelIndex idx;

    while (true)
    {
        i += offset;

        if (i >= model->rowCount())
            i = 0;
        else if (i < 0)
            i = model->rowCount() - 1;

        if (!tree->isRowHidden(i, idx))
        {
            for (SearchOpt searchOpt : filters)
            {
                for (COL col : searchOpt.m_keys)
                {
                    QModelIndex idx = model->index(i, col);
                    QString data;
                    if (col == COL::Value)
                    {
                        data = model->GetValueFullString(idx, true);
                    }
                    else if (col == COL::ART || col == COL::ErrorCode)
                    {
                        // Some columns only display their data in the tool tip
                        data = model->data(idx, Qt::ToolTipRole).toString();
                    }
                    else
                    {
                        // Most columns display their data
                        data = model->data(idx, Qt::DisplayRole).toString();
                    }
                    if (searchOpt.HasMatch(data))
                    {
                        tree->setCurrentIndex(idx);
                        QString msg = (filters.size() == 1) ?
                            QString("Found '%1' on line %2").arg(searchOpt.m_value, model->data(model->index(i, 0), Qt::DisplayRole).toString()) :
                            QString("Found a match on line %1").arg(model->data(model->index(i, 0), Qt::DisplayRole).toString());
                        statusBar()->showMessage(msg, 3000);
                        return;
                    }
                }
            }
        }

        if (i == start)
        {
            QString msg = (filters.size() == 1) ?
                QString("Not found: '%1'").arg(filters[0].m_value) :
                QString("No matching item.");
            statusBar()->showMessage(msg, 3000);
            return;
        }
    }
}

TreeModel * MainWindow::GetCurrentTreeModel()
{
    LogTab * logTab = GetCurrentLogTab();
    return logTab ? logTab->GetTreeModel() : nullptr;
}

QTreeView * MainWindow::GetCurrentTreeView()
{
    LogTab * logTab = GetCurrentLogTab();
    return logTab ? logTab->GetTreeView() : nullptr;
}

LogTab * MainWindow::GetLogTab(int index)
{
    return dynamic_cast<LogTab*>(tabWidget->widget(index));
}

LogTab * MainWindow::GetCurrentLogTab()
{
    return dynamic_cast<LogTab*>(tabWidget->currentWidget());
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == tabWidget->tabBar() &&
        event->type() == QEvent::MouseButtonPress)
    {
        auto mouseEvent = static_cast<QMouseEvent *>(event);
        int idx = tabWidget->tabBar()->tabAt(mouseEvent->pos());

        if (mouseEvent->button() == Qt::MouseButton::MiddleButton)
        {
            on_tabWidget_tabCloseRequested(idx);
            return true;
        }
        else if (mouseEvent->button() == Qt::RightButton)
        {
            LogTab * logTab = GetLogTab(idx);
            if (!logTab || logTab->GetTreeModel()->TabType() == TABTYPE::ExportedEvents)
                return true;

            QAction actionCopyFullPath("Copy full path", this);
            connect(&actionCopyFullPath, &QAction::triggered, logTab, &LogTab::CopyFullPath);

            QAction actionOpenDirectory("Show in folder", this);
            connect(&actionOpenDirectory, &QAction::triggered, logTab, &LogTab::ShowInFolder);

            QMenu tabBarMenu(this);
            tabBarMenu.addAction(&actionCopyFullPath);
            tabBarMenu.addAction(&actionOpenDirectory);
            tabBarMenu.exec(mouseEvent->globalPosition().toPoint());
            return true;
        }
    }

    return QMainWindow::eventFilter(obj, event);
}

