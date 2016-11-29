#include "mainwindow.h"

#include "finddlg.h"
#include "highlightdlg.h"
#include "logtab.h"
#include "options.h"
#include "optionsdlg.h"
#include "pathhelper.h"
#include "processevent.h"
#include "savefilterdialog.h"
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
#include <QTextStream>
#include <QTime>
#include <QTreeView>

MainWindow::MainWindow()
{
    setupUi(this);

    m_statusBar = new StatusBar(this);

    ReadSettings();
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
    if (!LoadLogFile(path))
    {
        RemoveRecentFile(path);
        Ui_MainWindow::menuRecent_files->removeAction(action);
    }
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

void MainWindow::UpdateMenuAndStatusBar()
{
    TreeModel * model = GetCurrentTreeModel();
    bool hasFilters = model && model->HasHighlightFilters();
    bool hasFindOpts = model && model->ValidFindOpts();

    // Menu items
    menuHighlight->setEnabled(model);
    menuFind->setEnabled(model);
    // QActions
    // File
    actionMerge_into_tab->setEnabled(model);
    actionClear_all_events->setEnabled(model);
    actionRefresh->setEnabled(model);
    actionShow_summary->setEnabled(model);
    actionOpen_timeline->setEnabled(model);
    actionClose_tab->setEnabled(model);
    actionClose_all_tabs->setEnabled(model);
    //Recent Files
    UpdateRecentFilesMenu();
    //Highlight
    actionFind_next_highlighted->setEnabled(hasFilters);
    actionFind_previous_highlighted->setEnabled(hasFilters);
    actionHighlight_only_mode->setEnabled(hasFilters);
    actionHighlight_only_mode->setChecked(model && model->m_highlightOnlyMode);
    menuLoad_filters->setEnabled(model);
    actionSave_filters->setEnabled(hasFilters);
    //Find
    actionFind_next->setEnabled(hasFindOpts);
    actionFind_previous->setEnabled(hasFindOpts);
    //Live capture
    actionTail_current_tab->setEnabled(model && model->TabType() != TABTYPE::ExportedEvents);
    actionTail_current_tab->setChecked(model && model->m_liveMode);

    // Status bar
    if (model == nullptr)
    {
        m_statusBar->SetRightLabelText("¯\\_(ツ)_/¯");
        return;
    }

    LogTab* currentTab = m_logTabs[model];
    if (currentTab)
    {
        currentTab->UpdateStatusBar();
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
        QTextStream in(&logfile);
        while (!in.atEnd())
        {
            auto line = in.readLine().trimmed();
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

void MainWindow::ExportEventsToTab(QModelIndexList list)
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

    connect(logTab, &LogTab::menuUpdateNeeded, this, &MainWindow::UpdateMenuAndStatusBar);
    connect(logTab, &LogTab::exportToTab, this, &MainWindow::ExportEventsToTab);
    int idx = tabWidget->addTab(logTab, "exported data");
    tabWidget->setCurrentIndex(idx);
    logTab->setFocus();

    QTreeView * exportedView = GetCurrentTreeView();
    TreeModel * exportedModel = GetCurrentTreeModel();
    exportedModel->SetTabType(TABTYPE::ExportedEvents);
    actionTail_current_tab->setEnabled(false);
    m_logTabs[exportedModel] = logTab;
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
        if(m_recentFiles.size() > 9)
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
        statusBar()->showMessage(QString("Unable to locate '%1'").arg(path), 3000);
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
    QTreeView * treeView = GetCurrentTreeView();
    statusBar()->showMessage(QString("%1 events loaded; %2 events skipped").arg(QString::number(treeView->model()->rowCount()), QString::number(skippedCount)), 3000);
    UpdateMenuAndStatusBar();
}

void MainWindow::on_actionTail_current_tab_triggered()
{
    QTreeView * view = GetCurrentTreeView();
    TreeModel * model = GetTreeModel(view);
    LogTab * currentTab = m_logTabs[model];

    if (m_logTabs.size() <= 0)
        return;

    if (actionTail_current_tab->isChecked())
    {
        if (currentTab->StartLiveCapture())
        {
            view->scrollToBottom();
            tabWidget->setTabIcon(tabWidget->currentIndex(), QIcon(":/tab-sync-thin.png"));
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
        statusBar()->showMessage(QString("Unable to locate '%1'").arg(path), 10000);
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
        CheckFileOpened(path);
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
        CheckFileOpened(directoryPath);
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

    auto tree = tabWidget->widget(idx)->findChild<QTreeView *>();
    TreeModel* model = GetTreeModel(tree);
    m_logTabs.insert(model, logTab);

    if (!isDirectory)
    {
        model->m_paths.append(path);
        model->SetTabType(TABTYPE::SingleFile);
        AddRecentFile(path);
    }
    else
    {
        model->SetTabType(TABTYPE::Directory);
    }
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

void MainWindow::CheckFileOpened(QString path)
{
    path.replace("\\", "/");
    for (int i = 0; i < m_logTabs.size(); i++)
    {
        QTreeView * view = GetTreeView(i);
        TreeModel * model = GetTreeModel(view);
        LogTab * currentTab = m_logTabs[model];
        if (currentTab && currentTab->GetTabPath().compare(path) == 0)
        {
            tabWidget->setCurrentIndex(i);
            currentTab->setFocus();
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
    // Use file path of the current tab as default directory.
    // It works with both file and directory paths.
    QString defaultDir;
    TreeModel* model = GetCurrentTreeModel();
    if (model)
    {
        LogTab* currentTab = m_logTabs[model];
        defaultDir = currentTab->GetTabPath();
    }

    QString directoryPath = QFileDialog::getExistingDirectory(this, "Select directory to monitor", defaultDir);
    if (!directoryPath.isEmpty())
    {
        StartDirectoryLiveCapture(directoryPath, "");
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

QTreeView* MainWindow::GetTreeView(int index)
{
    return tabWidget->widget(index)->findChild<QTreeView*>();
}

void MainWindow::on_tabWidget_tabCloseRequested(int index)
{
    if (index == -1)
        return;
    QTreeView * view = GetTreeView(index);
    TreeModel * model = GetTreeModel(view);
    LogTab * currentTab = m_logTabs[model];
    m_allFiles.removeAll(SystemCase(currentTab->GetTabPath()));
    currentTab->EndLiveCapture();
    m_logTabs.remove(model);

    QWidget* tabItem = tabWidget->widget(index);
    tabWidget->removeTab(index);
    delete(tabItem);
    tabItem = nullptr;
    UpdateMenuAndStatusBar();
}

void MainWindow::on_actionOpen_in_new_tab_triggered()
{
    QStringList files = QFileDialog::getOpenFileNames(this, "Select one or more log files to open", "", "Log Files (*.txt *.log);;All Files (*)");
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
    QStringList files = QFileDialog::getOpenFileNames(this, "Select one or more log files to merge into current tab", "", "Log Files (*.txt *.log);;All Files (*)");
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
        RefilterTreeView();
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

    RefilterTreeView();
    UpdateMenuAndStatusBar();
}

void MainWindow::on_actionClose_tab_triggered()
{
    on_tabWidget_tabCloseRequested(tabWidget->currentIndex());
}

void MainWindow::on_actionClose_all_tabs_triggered()
{
    for (int i = 0; i < m_logTabs.size(); i++)
    {
        QTreeView * view = GetTreeView(i);
        TreeModel * model = GetTreeModel(view);
        LogTab * currentTab = m_logTabs[model];
        currentTab->EndLiveCapture();
    }
    tabWidget->clear();
    m_logTabs.clear();
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

        RefilterTreeView();
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

    RefilterTreeView();
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

    // Open the dialog and see if ok was pressed. If so we want to update our model's _HighlightOpts member.
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
    OptionsDlg optionsDlg(this);
    optionsDlg.exec();
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

    SummaryCounter counters[] {
        { "Workbook opened", "command-post", "tabui:open-workbook", 0, nullptr },
        { "Query batch", "qp-batch-summary", nullptr, 0, nullptr },
        { "Query", "begin-query", nullptr, 0, nullptr },
        { "Query category", "begin-query", "query-category", 0, std::make_unique<CounterMap>() },
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

void MainWindow::on_actionOpen_timeline_triggered()
{
    QMessageBox timelineMsg(this);
    timelineMsg.setText("Timeline is currently not implemented for the TLV QT Port.");
    timelineMsg.exec();
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
    QTreeView * tree = GetCurrentTreeView();;
    TreeModel * model = GetTreeModel(tree);

    if (model->rowCount() == 0)
        return;

    const QVector<SearchOpt>& filters = (findHighlight) ?
        model->GetHighlightFilters() :
        QVector<SearchOpt> {model->m_findOpts};

    int start = tree->currentIndex().row();
    // If nothing is selected, the current index is -1. Force to start at 0 to avoid an infinite loop.
    if (start < 0)
    {
        start = 0;
    }
    int i = start;
    while (true)
    {
        i += offset;

        if (i >= model->rowCount())
            i = 0;
        else if (i < 0)
            i = model->rowCount() - 1;

        for (SearchOpt searchOpt : filters)
        {
            for (COL col : searchOpt.m_keys)
            {
                QModelIndex idx = model->index(i, col);
                QString data = (col == COL::Value) ?
                    model->GetValueFullString(idx, true) :
                    model->data(idx, Qt::DisplayRole).toString();
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

TreeModel* MainWindow::GetTreeModel(QTreeView* treeView)
{
    if (!treeView)
    {
        return nullptr;
    }
    return static_cast<TreeModel*>(treeView->model());
}

TreeModel * MainWindow::GetCurrentTreeModel()
{
    QTreeView * tree = GetCurrentTreeView();
    return GetTreeModel(tree);
}

QTreeView * MainWindow::GetCurrentTreeView()
{
    return tabWidget->currentWidget()->findChild <QTreeView *>();
}

void MainWindow::RefilterTreeView()
{
    auto view = GetCurrentTreeView();
    if (!view)
        return;

    QModelIndex previousIdx = view->currentIndex();

    view->setUpdatesEnabled(false);

    TreeModel* model = GetTreeModel(view);
    const int count = model->rowCount();
    const QModelIndex idx;

    for (int i = 0; i < count; i++)
    {
        bool hidden = model->m_highlightOnlyMode && !model->IsHighlightedRow(i);
        view->setRowHidden(i, idx, hidden);
    }

    view->setUpdatesEnabled(true);
    model->layoutChanged();
    view->scrollTo(previousIdx, QAbstractItemView::PositionAtCenter);
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == tabWidget->tabBar() &&
        event->type() == QEvent::MouseButtonPress)
    {
        auto mouseEvent = static_cast<QMouseEvent *>(event);
        int idx = tabWidget->tabBar()->tabAt(mouseEvent->pos());

        if (mouseEvent->button() == Qt::MidButton)
        {
            on_tabWidget_tabCloseRequested(idx);
            return true;
        }
        else if (mouseEvent->button() == Qt::RightButton)
        {
            TreeModel* model = GetTreeModel(GetTreeView(idx));
            if (!model || model->TabType() == TABTYPE::ExportedEvents)
                return true;

            LogTab* logTab = m_logTabs[model];

            QAction actionCopyFullPath("Copy full path", this);
            connect(&actionCopyFullPath, &QAction::triggered, logTab, &LogTab::CopyFullPath);

            QAction actionOpenDirectory("Show in folder", this);
            connect(&actionOpenDirectory, &QAction::triggered, logTab, &LogTab::ShowInFolder);

            QMenu tabBarMenu(this);
            tabBarMenu.addAction(&actionCopyFullPath);
            tabBarMenu.addAction(&actionOpenDirectory);
            tabBarMenu.exec(mouseEvent->globalPos());
            return true;
        }
    }

    return QMainWindow::eventFilter(obj, event);
}

