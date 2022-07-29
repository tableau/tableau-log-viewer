#include "logtab.h"
#include "ui_logtab.h"

#include "options.h"
#include "pathhelper.h"
#include "processevent.h"
#include "themeutils.h"
#include "treeitem.h"
#include "valuedlg.h"

#include <memory>
#include <initializer_list>
#include <QSet>
#include <QFontDatabase>
#include <QMenu>
#include <QJsonDocument>

LogTab::LogTab(QWidget *parent, StatusBar *bar, const EventListPtr events) :
    QWidget(parent),
    ui(new Ui::LogTab),
    m_bar(bar)
{
    ui->setupUi(this);
    setFocusProxy(ui->treeView);
    InitTreeView(events);
    InitMenus();
}

LogTab::~LogTab()
{
    delete ui;
}

void LogTab::InitTreeView(const EventListPtr events)
{
    ui->treeView->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);

    QStringList headers = QString("ID;File;Time;Elapsed;PID;TID;Severity;Request;Session;Site;User;Key;ART;Error Code;Value").split(";");

    // The parent of the model is this widget. The model will get destroyed when the widget is destroyed
    m_treeModel = new TreeModel(headers, events, this);
    ui->treeView->setModel(m_treeModel);

    m_bar->ShowMessage(QString("%1 events loaded").arg(QString::number(m_treeModel->rowCount())), 3000);

    // Display only time if all events occured on the same day
    bool multipleDays = false;
    if (events->size() >= 2) {
       QModelIndex idx = ui->treeView->currentIndex();
       auto firstDatetime = m_treeModel->index(0, COL::Time, idx.parent()).data(Qt::UserRole).toDateTime();
       auto lastDatetime = m_treeModel->index(m_treeModel->rowCount()-1, COL::Time, idx.parent()).data(Qt::UserRole).toDateTime();
       multipleDays = firstDatetime.date() != lastDatetime.date();
    }
    m_treeModel->SetTimeMode(multipleDays ? TimeMode::GlobalDateTime : TimeMode::GlobalTime);

    bool hasNoKey = (events->size() > 0 && events->at(0)["k"].toString().isEmpty());

    SetColumn(COL::ID, 80, false);
    SetColumn(COL::File, 110, true);
    SetColumn(COL::Time, 190, hasNoKey);
    SetColumn(COL::Elapsed, 50, hasNoKey);
    SetColumn(COL::PID, 30, true);
    SetColumn(COL::TID, 50, true);
    SetColumn(COL::Severity, 50, true);
    SetColumn(COL::Request, 30, true);
    SetColumn(COL::Session, 30, true);
    SetColumn(COL::Site, 180, true);
    SetColumn(COL::User, 30, true);
    SetColumn(COL::Key, 120, hasNoKey);
    SetColumn(COL::ART, 30, hasNoKey);
    SetColumn(COL::ErrorCode, 30, hasNoKey);
    ui->treeView->SetAutoResizeColumns({COL::Time, COL::Elapsed});

    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeView->header()->setContextMenuPolicy(Qt::CustomContextMenu);

    // Connect slots
    connect(ui->treeView, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(RowDoubleClicked(QModelIndex)));
    connect(ui->treeView, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(RowRightClicked(QPoint)));
    connect(ui->treeView->header(), SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(HeaderRightClicked(QPoint)));
}

void LogTab::SetColumn(COL column, int width, bool isHidden)
{
    ui->treeView->setColumnWidth(column, width);
    ui->treeView->setColumnHidden(column, isHidden);
}

void LogTab::InitMenus()
{
    using namespace ThemeUtils;

    m_hideSelectedEvent = new QAction(QIcon(GetThemedIcon(":/ctx-hide.png")), "Hide selected event", this);
    connect(m_hideSelectedEvent, &QAction::triggered, this, &LogTab::RowHideSelected);

    m_hideSelectedType = new QAction(QIcon(GetThemedIcon(":/ctx-hide.png")), "Hide all events of this type", this);
    connect(m_hideSelectedType, &QAction::triggered, this, &LogTab::RowHideSelectedType);

    // The columns available in the `Highlight` and `Export` submenus
    std::initializer_list<std::pair<const char*,COL>> availableFilterColumns{
        {"This type",COL::Key},
        {"This session",COL::Session},
        {"This request",COL::Request},
        {"This thread",COL::TID},
        {"This severity",COL::Severity}
    };

    // Top-level `Export events`
    m_exportToTabAction = new QAction(QIcon(GetThemedIcon(":/ctx-newtab.png")), "Export event(s) to new tab", this);
    connect(m_exportToTabAction, &QAction::triggered, [this]() { ExportToNewTab(); });
    // Populate `Export to tab` menu
    m_exportToTabMenu = new QMenu("Export to new tab",this);
    m_exportToTabMenu->setIcon(QIcon(GetThemedIcon(":/ctx-newtab.png")));
    for (auto& e : availableFilterColumns){
        auto label = e.first;
        auto column = e.second;
        QAction* exportSelectedType = new QAction(QIcon(GetThemedIcon(":/ctx-newtab.png")), label, m_exportToTabMenu);
        connect(exportSelectedType, &QAction::triggered, [this,column](){ ExportToNewTab(column); });
        m_exportToTabMenu->addAction(exportSelectedType);
    }

    // Top-level `Highlight all events ofselected types`
    m_highlightSelectedType = new QAction(QIcon(GetThemedIcon(":/ctx-highlight.png")), "Highlight all events of this type", this);
    connect(m_highlightSelectedType, &QAction::triggered, [this]() { RowHighlightSelected(COL::Key); });
    // Populate the `Highlight all events of` submenu
    m_highlightSelectedMenu = new QMenu("Highlight all events of",this);
    m_highlightSelectedMenu->setIcon(QIcon(GetThemedIcon(":/ctx-highlight.png")));
    for (auto& e : availableFilterColumns){
        auto label = e.first;
        auto column = e.second;
        QAction* highlightSelectedType = new QAction(QIcon(GetThemedIcon(":/ctx-highlight.png")), label, m_highlightSelectedMenu);
        connect(highlightSelectedType, &QAction::triggered, [this,column](){ RowHighlightSelected(column); });
        m_highlightSelectedMenu->addAction(highlightSelectedType);
    }

    m_copyItemsHtmlAction = new QAction(QIcon(GetThemedIcon(":/ctx-copy.png")), "Copy as HTML", this);
    m_copyItemsHtmlAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_C));
    connect(m_copyItemsHtmlAction, &QAction::triggered, this, &LogTab::CopyItemDetailsAsHtml);

    m_copyItemsTextAction = new QAction(QIcon(GetThemedIcon(":/ctx-copy.png")), "Copy as text", this);
    connect(m_copyItemsTextAction, &QAction::triggered, this, &LogTab::CopyItemDetailsAsText);

    m_copyItemsNormalizedTextAction = new QAction(QIcon(GetThemedIcon(":/ctx-copy.png")), "Copy as normalized text", this);
    connect(m_copyItemsNormalizedTextAction, &QAction::triggered, this, &LogTab::CopyItemDetailsAsNormalizedText);

    InitOneRowMenu();
    InitTwoRowsMenu();
    InitMultipleRowsMenu();
    InitHeaderMenu();
}

void LogTab::InitTwoRowsMenu()
{
    using namespace ThemeUtils;
    m_twoRowsMenu = new QMenu(this);
    m_twoRowsMenu->addAction(QIcon(GetThemedIcon(":/ctx-book.png")), "Diff selected events",
                             this, &LogTab::RowDiffEvents, QKeySequence(Qt::CTRL | Qt::Key_D));
    m_twoRowsMenu->addAction(m_hideSelectedEvent);
    m_twoRowsMenu->addAction(m_hideSelectedType);
    m_twoRowsMenu->addSeparator();
    m_twoRowsMenu->addAction(m_highlightSelectedType);
    m_twoRowsMenu->addMenu(m_highlightSelectedMenu);
    m_twoRowsMenu->addSeparator();
    m_twoRowsMenu->addAction(m_exportToTabAction);
    m_twoRowsMenu->addMenu(m_exportToTabMenu);
    m_twoRowsMenu->addSeparator();
    m_twoRowsMenu->addAction(m_copyItemsHtmlAction);
    m_twoRowsMenu->addAction(m_copyItemsTextAction);
    m_twoRowsMenu->addAction(m_copyItemsNormalizedTextAction);
}

void LogTab::InitMultipleRowsMenu()
{
    m_multipleRowsMenu = new QMenu(this);
    m_multipleRowsMenu->addAction(m_hideSelectedEvent);
    m_multipleRowsMenu->addAction(m_hideSelectedType);
    m_multipleRowsMenu->addSeparator();
    m_multipleRowsMenu->addAction(m_highlightSelectedType);
    m_multipleRowsMenu->addMenu(m_highlightSelectedMenu);
    m_multipleRowsMenu->addSeparator();
    m_multipleRowsMenu->addAction(m_exportToTabAction);
    m_multipleRowsMenu->addMenu(m_exportToTabMenu);
    m_multipleRowsMenu->addSeparator();
    m_multipleRowsMenu->addAction(m_copyItemsHtmlAction);
    m_multipleRowsMenu->addAction(m_copyItemsTextAction);
    m_multipleRowsMenu->addAction(m_copyItemsNormalizedTextAction);
}

void LogTab::InitOneRowMenu()
{
    using namespace ThemeUtils;

    QAction *findPrevSelectedType = new QAction(QIcon(GetThemedIcon(":/ctx-up.png")), "Find previous event of this type", this);
    connect(findPrevSelectedType, &QAction::triggered, this, &LogTab::RowFindPrevSelectedType);

    QAction *findNextSelectedType = new QAction(QIcon(GetThemedIcon(":/ctx-down.png")), "Find next event of this type", this);
    connect(findNextSelectedType, &QAction::triggered, this, &LogTab::RowFindNextSelectedType);

    m_showGlobalDateTime = new QAction(QIcon(GetThemedIcon(":/ctx-datetime.png")), "Show global date && time", this);
    m_showGlobalDateTime->setCheckable(true);
    connect(m_showGlobalDateTime, &QAction::triggered, this, &LogTab::RowShowGlobalDateTime);

    m_showGlobalTime = new QAction(QIcon(GetThemedIcon(":/ctx-time.png")), "Show global time", this);
    m_showGlobalTime->setCheckable(true);
    connect(m_showGlobalTime, &QAction::triggered, this, &LogTab::RowShowGlobalTime);

    m_showTimeDeltas = new QAction(QIcon(GetThemedIcon(":/ctx-time.png")), "Show time deltas to this event", this);
    m_showTimeDeltas->setCheckable(true);
    connect(m_showTimeDeltas, &QAction::triggered, this, &LogTab::RowShowTimeDeltas);

    QAction *actionOpenFile = new QAction(QIcon(GetThemedIcon(":/ctx-open-file-tab.png")), "Open log file in new tab", this);
    connect(actionOpenFile, &QAction::triggered, this, &LogTab::OpenSelectedFile);

    QActionGroup * group = new QActionGroup(this);
    group->addAction(m_showGlobalDateTime);
    group->addAction(m_showGlobalTime);
    group->addAction(m_showTimeDeltas);
    group->setExclusive(true);

    m_oneRowMenu = new QMenu(this);
    m_oneRowMenu->addAction(m_hideSelectedEvent);
    m_oneRowMenu->addAction(m_hideSelectedType);
    m_oneRowMenu->addSeparator();
    m_oneRowMenu->addAction(findNextSelectedType);
    m_oneRowMenu->addAction(findPrevSelectedType);
    m_oneRowMenu->addAction(m_highlightSelectedType);
    m_oneRowMenu->addMenu(m_highlightSelectedMenu);
    m_oneRowMenu->addSeparator();
    m_oneRowMenu->addAction(m_exportToTabAction);
    m_oneRowMenu->addMenu(m_exportToTabMenu);
    m_oneRowMenu->addSeparator();
    m_oneRowMenu->addAction(m_showGlobalDateTime);
    m_oneRowMenu->addAction(m_showGlobalTime);
    m_oneRowMenu->addAction(m_showTimeDeltas);
    m_oneRowMenu->addSeparator();
    m_oneRowMenu->addAction(actionOpenFile);
    m_openFileMenuIdx = m_oneRowMenu->actions().size() - 1;
    m_oneRowMenu->addSeparator();
    m_oneRowMenu->addAction(m_copyItemsHtmlAction);
    m_oneRowMenu->addAction(m_copyItemsTextAction);
    m_oneRowMenu->addAction(m_copyItemsNormalizedTextAction);
}

void LogTab::keyPressEvent(QKeyEvent *event)
{
    auto idxList =  ui->treeView->selectionModel()->selectedRows();
    auto rowCount = idxList.count();

    switch (event->key())
    {
    case Qt::Key_Enter:
    case Qt::Key_Return:
        if (rowCount == 1)
            ShowItemDetails(idxList[0]);
        break;
    case Qt::Key_Backspace:
    case Qt::Key_Delete:
        RowHideSelected();
        break;
    default:
        // Check keys with modifiers together and ensure regular key events get propagated.
        if ((event->key() == Qt::Key_C) &&
            (QApplication::keyboardModifiers() & Qt::ControlModifier) &&
            (rowCount > 0))
        {
            CopyItemDetailsAsHtml();
        }
        else if ((event->key() == Qt::Key_D) &&
                 (QApplication::keyboardModifiers() & Qt::ControlModifier) &&
                 (rowCount == 2))
        {
            RowDiffEvents();
        }
        else if ((event->key() == Qt::Key_I) &&
                 (QApplication::keyboardModifiers() & Qt::ControlModifier) &&
                 (QApplication::keyboardModifiers() & Qt::ShiftModifier))
        {
            QMessageBox msgBox(this);
            msgBox.setWindowTitle("Debug Info");
            msgBox.setText(GetDebugInfo());
            msgBox.setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
            msgBox.exec();
        }
        else
        {
            QWidget::keyPressEvent(event);
        }
    }
}

void LogTab::SetTabPath(const QString& path)
{
    m_tabPath = path;
    if (m_treeModel->TabType() == TABTYPE::Directory)
    {
        m_liveDirectory = std::make_unique<QDir>(path);
        m_liveDirectory->setNameFilters(QStringList({"*.txt", "*.log"}));
    }
    else if (m_treeModel->TabType() == TABTYPE::SingleFile)
    {
        m_logFile.setFileName(path);
    }
}

QString LogTab::GetTabPath() const
{
    return m_tabPath;
}

void LogTab::SetUpTimer()
{
    if (m_treeModel->TabType() == TABTYPE::Directory)
    {
        connect(&m_timer, &QTimer::timeout, this, &LogTab::ReadDirectoryFiles);
    }
    else if (m_treeModel->TabType() == TABTYPE::SingleFile)
    {
        connect(&m_timer, &QTimer::timeout, this, &LogTab::ReadFile);
    }
    m_timer.start(250);
}

void LogTab::SetUpFile(std::shared_ptr<QFile> file)
{
    if (!file->exists())
    {
        QErrorMessage errorDialog(this);
        errorDialog.showMessage("File doesn't exist");
        errorDialog.exec();
        return;
    }
    if (!file->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QErrorMessage errorDialog(this);
        errorDialog.showMessage("File could not be opened");
        errorDialog.exec();
        return;
    }

    QByteArray line;
    while (!file->atEnd())
    {
        line = file->readLine();
        if (line.isEmpty() || line.startsWith("\n"))
        {
            continue;
        }
        break;
    }

    bool includeAllTextFiles = Options::GetInstance().getCaptureAllTextFiles();
    if (includeAllTextFiles || line.startsWith("{"))
    {
        int offset = file->size();
        file->seek(offset);
        m_directoryFiles[file->fileName()] = file;
    }
    else
    {
        file->close();
        m_excludedFileNames.append(file->fileName());
    }
}

void LogTab::StartDirectoryLiveCapture()
{
    SetColumn(COL::File, 110, false);
    m_eventIndex = 1;
    m_treeModel->m_liveMode = true;
    QStringList files = m_liveDirectory->entryList(QDir::Files);
    for (const QString& fileName : files)
    {
        QString fullPath = m_liveDirectory->path() + "/" + fileName;
        std::shared_ptr<QFile> file = std::make_shared<QFile>(fullPath);
        SetUpFile(file);
    }
    SetUpTimer();
}

void LogTab::ReadDirectoryFiles()
{
    QStringList files = m_liveDirectory->entryList(QDir::Files);
    if (files.size() > m_directoryFiles.size() + m_excludedFileNames.size())
    {
        for (const QString& fileName : files)
        {
            QString fullPath = m_liveDirectory->path() + "/" + fileName;
            if (!m_directoryFiles.keys().contains(fullPath) && !m_excludedFileNames.contains(fullPath))
            {
                std::shared_ptr<QFile> file = std::make_shared<QFile>(fullPath);
                SetUpFile(file);
            }
        }
    }

    EventList newEvents;
    for (QString filePath : m_directoryFiles.keys())
    {
        std::shared_ptr<QFile> file = m_directoryFiles[filePath];
        if (file->pos() > file->size())
        {
            file->seek(0);
        }
        QStringList filePathList = filePath.split("/");
        auto& fileName = filePathList.at(filePathList.length() - 1);
        while (!file->atEnd())
        {
            auto line = file->readLine().trimmed();
            if (line.isEmpty())
            {
                continue;
            }
            QJsonObject jsonObj = ProcessEvent::ProcessLogEventMessage(m_eventIndex, line, fileName);
            if (jsonObj.isEmpty())
            {
                continue;
            }
            newEvents.append(jsonObj);
            m_eventIndex++;
        }
    }

    if (newEvents.count() > 0)
    {
        bool firstEntries = (m_treeModel->rowCount() == 0);
        int startRow = m_treeModel->MergeIntoModelData(newEvents);
        newEvents.clear();

        if (m_treeModel->m_highlightOnlyMode)
        {
            const QModelIndex idx;
            for (int i = startRow; i < m_treeModel->rowCount(); i++)
            {
                bool hidden = !m_treeModel->IsHighlightedRow(i);
                ui->treeView->setRowHidden(i, idx, hidden);
            }
        }

        TrimEventCount();
        UpdateModelView();
        if (firstEntries)
        {
            ui->treeView->ResizeColumns();
        }
    }
}

void LogTab::UpdateModelView()
{
    if (ui->treeView->verticalScrollBar()->value() == ui->treeView->verticalScrollBar()->maximum())
    {
        ui->treeView->scrollToBottom();
    }
}

bool LogTab::StartFileLiveCapture()
{
    QModelIndex idx = m_treeModel->index(m_treeModel->rowCount() - 1, 0);
    auto item_model = idx.model();
    if (item_model)
    {
        auto idx_info = item_model->index(idx.row(), COL::ID, idx.parent());
        m_eventIndex = idx_info.data().toInt() + 1;
    }
    else
    {
        m_eventIndex = 1;
    }

    // Open file/check file exists and is readable
    if (!m_logFile.exists())
    {
        QErrorMessage errorDialog(this);
        errorDialog.showMessage("File doesn't exist");
        errorDialog.exec();
        return false;
    }
    if (!m_logFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QErrorMessage errorDialog(this);
        errorDialog.showMessage("File could not be opened");
        errorDialog.exec();
        return false;
    }
    int offset = m_logFile.size();
    m_logFile.seek(offset);

    // Set up timer to update/read lines
    SetUpTimer();
    m_treeModel->m_liveMode = true;
    return true;
}

void LogTab::ReadFile()
{
    const QModelIndex idx;
    EventList newEvents;
    if (m_logFile.pos() > m_logFile.size())
    {
        m_logFile.seek(0);
    }
    while (!m_logFile.atEnd())
    {
        auto line = m_logFile.readLine().trimmed();
        if (line.isEmpty())
        {
            continue;
        }
        QJsonObject jsonObj = ProcessEvent::ProcessLogEventMessage(m_eventIndex, line, m_logFile.fileName());
        if (jsonObj.isEmpty())
        {
            continue;
        }
        newEvents.append(jsonObj);
        m_treeModel->AddToModelData(newEvents);
        newEvents.clear();

        int offset = m_treeModel->rowCount() - 1;
        bool hidden = m_treeModel->m_highlightOnlyMode && !m_treeModel->IsHighlightedRow(offset);
        ui->treeView->setRowHidden(offset, idx, hidden);

        m_eventIndex++;
    }
    TrimEventCount();
    UpdateModelView();
}

bool LogTab::StartLiveCapture()
{
    if (!m_treeModel)
        return false;

    if (m_treeModel->m_liveMode)
        return true;

    if (m_treeModel->TabType() == TABTYPE::SingleFile)
    {
        return StartFileLiveCapture();
    }
    else if (m_treeModel->TabType() == TABTYPE::Directory)
    {
        StartDirectoryLiveCapture();
        return true;
    }

    return false;
}

void LogTab::EndLiveCapture()
{
    if (m_treeModel != nullptr && m_treeModel->m_liveMode)
    {
        m_treeModel->m_liveMode = false;
        if (m_treeModel->TabType() == TABTYPE::SingleFile)
        {
            m_logFile.close();
        }
        else if (m_treeModel->TabType() == TABTYPE::Directory)
        {
            for (std::shared_ptr<QFile> file : m_directoryFiles)
            {
                file->close();
            }
            m_directoryFiles.clear();
            m_excludedFileNames.clear();
        }
        m_timer.stop();
    }
}

void LogTab::TrimEventCount()
{
    const static int MaxEventCount = 100000;
    int rowCount = m_treeModel->rowCount();
    if (rowCount > MaxEventCount)
    {
        m_treeModel->removeRows(0, rowCount - MaxEventCount);
    }
}

void LogTab::RowDoubleClicked(const QModelIndex& idx)
{
    ShowItemDetails(idx);
}

void LogTab::ShowDetails(const QModelIndex& idx, ValueDlg& valueDlg)
{
    auto item_model = idx.model();
    auto idx_key = item_model->index(idx.row(), COL::Key, idx.parent());
    auto idx_id = item_model->index(idx.row(), COL::ID, idx.parent());
    auto id = idx_id.data().toString();
    auto key = idx_key.data().toString();
    auto eventContent = m_treeModel->GetConsolidatedEventContent(idx);
    valueDlg.SetContent(id,key,eventContent);
}

void LogTab::ShowItemDetails(const QModelIndex& idx)
{
    ValueDlg valueDlg(this);
    m_valueDlg = &valueDlg;
    connect(&valueDlg, &ValueDlg::next, this, &LogTab::ChangeNextIndex);
    connect(&valueDlg, &ValueDlg::prev, this, &LogTab::ChangePrevIndex);
    ShowDetails(idx, valueDlg);
    // change selection mode so that when "Ctrl-up" or "Ctrl-down" is used
    // for next/previous, items don't stay highlighted after they
    // are no longer selected/being viewed
    ui->treeView->setSelectionMode(QAbstractItemView::ContiguousSelection);
    valueDlg.exec();
    ui->treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
}

void LogTab::ChangeNextIndex()
{
    int row = ui->treeView->currentIndex().row() + 1;
    int column = ui->treeView->currentIndex().column();
    if (m_treeModel->m_highlightOnlyMode)
    {
        while (!m_treeModel->IsHighlightedRow(row) && row < m_treeModel->rowCount())
        {
            row++;
        }
    }
    if (row < m_treeModel->rowCount())
    {
        QModelIndex idx = m_treeModel->index(row, column);
        ui->treeView->setCurrentIndex(idx);
        ShowDetails(idx, *m_valueDlg);
    }
}

void LogTab::ChangePrevIndex()
{
    int row = ui->treeView->currentIndex().row() - 1;
    int column = ui->treeView->currentIndex().column();
    if (m_treeModel->m_highlightOnlyMode)
    {
        while (!m_treeModel->IsHighlightedRow(row) && row >= 0)
        {
            row--;
        }
    }
    if (row >= 0)
    {
        QModelIndex idx = m_treeModel->index(row, column);
        ui->treeView->setCurrentIndex(idx);
        ShowDetails(idx, *m_valueDlg);
    }
}

void NormalizeLogText(QString& logText)
{
    std::vector<std::pair<QString, QString>> replacements
    {
        {"id: (-)?[\\w+/]+,", "id: ID,"},
        {"root: (-)?[\\w+/]+,", "root: ID,"},
        {"sponsor: (-)?[\\w+/]+,", "sponsor: ID,"},
        {"{ e: \\d+, i: \\d+", "{ e: N, i: N"},
        {" ne: \\d+, ni: \\d+", " ne: N, ni: N"},
        {" peak: \\d+", " peak: N"},
        {"pid=\\d+", "pid=N"},
        {"elapsed: \\d+(.\\d+)?", "elapsed: N.N"},
        {"elapsed=\\d+.\\d+", "elapsed=N.N"},
        {"elapsed-ms: \\d+", "elapsed-ms: N"},
        {"elapsedMs: \\d+", "elapsedMs: N"},
        {"elapsed-([\\w-]+): \\d+(.\\d+)?", "elapsed-\\1: N.N"},
        {":thread-session: \\d+", ":thread-session: N"},
        {":thread-session: \\(MISMATCH\\) \\d+", ":thread-session: (MISMATCH) N;"},
        {":thread-session='\\d+'", ":thread-session='N'"},
        {"this: 0x\\w+", "this: 0x0N"},
        {"query-hash: \\d+", "query-hash: N"},
        {"\\w:[\\\\/\\w]+[\\\\/]TableauTemp[\\\\/]", "TableauTemp/"},
        {"[/\\w]+/tableau-temp/", "TableauTemp/"},
        {"TableauTemp/[f]?tde_\\w{28}", "TableauTemp/tde_ID"},
        {"federation_[\\w_]+.hyper", "federation_ID.hyper"},
        {"#TableauTemp_[\\w_]+.hyper", "#TableauTemp_ID.hyper"},
        {"federation_\\w{28}", "federation_ID"},
        {"[0-9A-F]{8}-[0-9A-F]{4}-[0-9A-F]{4}-[0-9A-F]{4}-[0-9A-F]{12}", "GUID"},
        {"#Tableau_[\\w_]+_Connect", "#Tableau_ID_Connect"},
        {"#Tableau_\\d+_\\d+", "#Tableau_N_N"},
        {"#Tableau_\\d+_[\\d\\w-]+_", "#Tableau_N_ID_"},
        {"Tableau_\\d+_GUID_\\d+", "Tableau_N_GUID_N"},
        {"FQ_Temp_\\d+", "FQ_Temp_N"},
        {"[0-9]{1,2}/[0-9]{1,2}/[0-9]{2,4} [0-9]{1,2}:[0-9]{1,2}:[0-9]{1,2} [AP]M", "TIMESTAMP"},
        {"(\")?tab.domain:\\S+(\")?", "\"tab.endpoint\""},
        {"\"tab.pipe:\\S+\"", "\"tab.endpoint\""},
        {"protocol: [0-9a-f]{12};", "protocol: N;"},
        {"port='\\d+'", "port='N'"},
    };

    for (const auto& repl : replacements)
    {
        logText.replace(QRegularExpression(repl.first), repl.second);
    }
}

void LogTab::CopyItemDetailsAsHtml() const
{
    CopyItemDetails(false, false);
}

void LogTab::CopyItemDetailsAsText() const
{
    CopyItemDetails(true, false);
}

void LogTab::CopyItemDetailsAsNormalizedText() const
{
    CopyItemDetails(true, true);
}

void LogTab::CopyItemDetails(bool textOnly, bool normalized) const
{
    QModelIndexList idxList = ui->treeView->selectionModel()->selectedRows();
    int columnCount = m_treeModel->columnCount();
    if (!columnCount || !idxList.count())
        return;

    QString copyText;
    QString copyHtml =
        "<html><head><style>"
        "table, th, td { border: 1px solid black; border-collapse: collapse; } "
        "th, td { font-family: Monaco, Consolas, Monospace; font-size: 10pt; text-align: left; }"
        "</style></head><body><table>\n<tr>";

    // Header row
    for (int i = 0; i < columnCount; i++)
    {
        // Only copy columns that are not hidden. And in normalized mode, only copy Key and Value columns as others have
        // mostly transient values that are not useful for comparisons.
        if (!(ui->treeView->isColumnHidden(i)) && (!normalized || (i == COL::Key) || (i == COL::Value)))
        {
            QString info = m_treeModel->headerData(i, Qt::Horizontal).toString();
            copyText += info + "\t";
            copyHtml += "<th>" + info + "</th> ";
        }
    }
    copyText[copyText.length() - 1] = '\n';
    copyHtml += "</tr>\n";

    for (const auto& idx : idxList)
    {
        auto item_model = idx.model();
        copyHtml += "<tr>";
        for (int i = 0; i < columnCount; i++)
        {
            if (!(ui->treeView->isColumnHidden(i)) && (!normalized || (i == COL::Key) || (i == COL::Value)))
            {
                auto idx_info = item_model->index(idx.row(), i, idx.parent());
                QString info = (i == COL::Value) ?
                    m_treeModel->GetValueFullString(idx, true).replace("\t", " ") :
                    idx_info.data().toString();

                if (info.length() > 0)
                {
                    copyText += info + "\t";
                    copyHtml += "<td>" + info + "</td> ";
                }
                else
                {
                    copyText += "-\t";
                    copyHtml += "<td>-</td> ";
                }
            }
        }
        copyText[copyText.length() - 1] = '\n';
        copyHtml += "</tr>\n";
    }

    copyHtml += "</table></body></html>";

    QClipboard* clipboard = QApplication::clipboard();
    auto mime = std::make_unique<QMimeData>();
    if (!textOnly)
    {
        mime->setText(copyText);
        mime->setHtml(copyHtml);
    }
    else
    {
        if (normalized)
            NormalizeLogText(copyText);
        mime->setText(copyText);
    }
    clipboard->setMimeData(mime.release());
}

void LogTab::ExportToNewTab()
{
    auto idxList = ui->treeView->selectionModel()->selectedRows();
    // Sort selected rows by row number to preserve the event ordering.
    // The selection model saves them in the order they were clicked
    // on, which might not be the same as the order that the events occurred in
    std::sort(idxList.begin(), idxList.end(), [](QModelIndex a, QModelIndex b) {
        return a.row() < b.row();
    });
    emit exportToTab(idxList,"exported data");
}

void LogTab::ExportToNewTab(COL column)
{
    // Generate the list of selected values
    auto selectionList = ui->treeView->selectionModel()->selectedRows();
    QSet<QString> exportedValues;
    for (const auto& selected : selectionList) {
       QString value = selected.model()->index(selected.row(), column, selected.parent()).data().toString();
       exportedValues.insert(value);
    }

    // Build the name for the new tab
    QStringList sortedValues = QStringList(exportedValues.begin(), exportedValues.end());
    sortedValues.sort();
    QString name = QString(GetColumnName(column)) + " " + sortedValues.join(",");

    // Generate the index list
    const int count = m_treeModel->rowCount();
    const QModelIndex root;
    QModelIndexList exportedIdxList;
    for (int i = 0; i < count; ++i)
    {
        auto idx = m_treeModel->index(i, column, root);
        auto value = idx.data().toString();
        if (exportedValues.contains(value)) {
            exportedIdxList.append(idx);
        }
    }

    emit exportToTab(exportedIdxList, name);
}

void LogTab::OpenSelectedFile()
{
    auto idxList = ui->treeView->selectionModel()->selectedRows();
    if (idxList.size() < 1)
        return;

    QModelIndex idx = idxList[0];
    QString fileName = idx.model()->index(idx.row(), COL::File, idx.parent()).data().toString();
    QString filePath = this->m_tabPath + "/" + fileName;
    emit openFile(filePath);
}

void LogTab::RowRightClicked(const QPoint& pos)
{
    auto idxList =  ui->treeView->selectionModel()->selectedRows();
    auto rowCount = idxList.count();
    m_exportToTabAction->setEnabled(true);
    if (rowCount == 0)
    {
        m_exportToTabAction->setEnabled(false);
    }
    else if (rowCount == 1)
    {
        bool isDirectoryTab = (m_treeModel->TabType() == TABTYPE::Directory);
        auto menuActions = m_oneRowMenu->actions();
        menuActions[m_openFileMenuIdx]->setVisible(isDirectoryTab);

        TimeMode timeMode = m_treeModel->GetTimeMode();
        m_showGlobalDateTime->setChecked(timeMode==TimeMode::GlobalDateTime);
        m_showGlobalTime->setChecked(timeMode==TimeMode::GlobalTime);
        m_showTimeDeltas->setChecked(timeMode==TimeMode::TimeDeltas);

        m_oneRowMenu->popup(ui->treeView->viewport()->mapToGlobal(pos));
    }
    else if (rowCount == 2)
    {
        m_twoRowsMenu->popup(ui->treeView->viewport()->mapToGlobal(pos));
    }
    else
    {
        m_multipleRowsMenu->popup(ui->treeView->viewport()->mapToGlobal(pos));
    }
}

void LogTab::RowDiffEvents()
{
    auto idxList =  ui->treeView->selectionModel()->selectedRows();
    // We would only reach here if idxList.size() == 2
    QModelIndex firstIdx = idxList[0];
    QModelIndex secondIdx = idxList[1];

    QString firstVal = m_treeModel->GetValueFullString(firstIdx);
    QString secondVal= m_treeModel->GetValueFullString(secondIdx);

    // Save the two strings into temp files
    std::unique_ptr<QTemporaryFile> firstFile = std::make_unique<QTemporaryFile>();
    std::unique_ptr<QTemporaryFile> secondFile = std::make_unique<QTemporaryFile>();

    QTextStream firstOut(firstFile.get());
    QTextStream secondOut(secondFile.get());

    firstFile->open();
    secondFile->open();

    firstOut << firstVal << "\n";
    secondOut << secondVal << "\n";

    firstOut.flush();
    secondOut.flush();

    // Initialize the diff tool
    QString diffToolPath = Options::GetInstance().getDiffToolPath();
    if (QSysInfo::productType() == "osx" &&
        diffToolPath.endsWith(".app"))
    {
        // In Mac, if the tool is an app, we need to use "open"
        diffToolPath = QString("open -a \"%1\" --args").arg(diffToolPath);
    }
    else
    {
        // Put quotes around the path, in case it has any spaces
        diffToolPath = QString("\"%1\"").arg(diffToolPath);
    }
    QString diffCommand = QString("%1 \"%2\" \"%3\"").arg(
        diffToolPath,
        firstFile->fileName(),
        secondFile->fileName());

    qDebug() << "Diff tool path: " << diffCommand;
    if(!QProcess::startDetached(diffCommand))
    {
        QMessageBox::information(this, tr("Diff Error!"), tr("Cannot find diff tool. Please set it in application options menu."));
    }

    // We are storing the temp files in a vector. Once the vector goes out of scope, the QTemporaryFiles are automatically deleted.
    // Temp files are under AppData\Local\Temp, but are automitically deleted when the tab or tlv is closed. They remain if TLV crashes.
    m_tempFiles.push_back(std::move(firstFile));
    m_tempFiles.push_back(std::move(secondFile));
}

void LogTab::RowHideSelected()
{
    ui->treeView->setUpdatesEnabled(false);
    auto idxList = ui->treeView->selectionModel()->selectedRows();

    // Sort the indices in reverse order. Remove the items from the
    // end first to make sure indices stay valid after each removal
    std::sort(idxList.rbegin(), idxList.rend());

    for (const auto& idx : idxList) {
       m_treeModel->removeRow(idx.row(), idx.parent());
    }
    ui->treeView->setUpdatesEnabled(true);
    menuUpdateNeeded();
}

void LogTab::RowHideSelectedType()
{
    ui->treeView->setUpdatesEnabled(false);
    auto idxList = ui->treeView->selectionModel()->selectedRows();
    QSet<QString> hiddenKeys;
    for (const auto& idx : idxList) {
       QString key = idx.model()->index(idx.row(), COL::Key, idx.parent()).data().toString();
       hiddenKeys.insert(key);
    }
    int count = 0;
    for (const auto& key : hiddenKeys) {
       auto match = m_treeModel->match(m_treeModel->index(0, COL::Key), Qt::DisplayRole, key, 1, Qt::MatchExactly);
       while (match.length() > 0)
       {
           m_treeModel->removeRow(match[0].row());
           match = m_treeModel->match(m_treeModel->index(match[0].row(), COL::Key), Qt::DisplayRole, key, 1, Qt::MatchExactly);
           ++count;
       }
    }
    ui->treeView->setUpdatesEnabled(true);
    menuUpdateNeeded();

    if (hiddenKeys.count() == 1) {
        QString key = hiddenKeys.values()[0];
        m_bar->ShowMessage(QString("%1 '%2' event(s) hidden").arg(QString::number(count), key), 3000);
    } else {
        m_bar->ShowMessage(QString("%1 event(s) of %2 types hidden").arg(QString::number(count), QString::number(hiddenKeys.count())), 3000);
    }
}

void LogTab::RowFindNextSelectedType()
{
    QModelIndex idx = ui->treeView->currentIndex();

    m_treeModel->m_findOpts.m_keys.clear();
    m_treeModel->m_findOpts.m_keys.append(COL::Key);
    m_treeModel->m_findOpts.m_value = idx.model()->index(idx.row(), COL::Key, idx.parent()).data().toString();
    RowFindNext();
    menuUpdateNeeded();
}

void LogTab::RowFindPrevSelectedType()
{
    QModelIndex idx = ui->treeView->currentIndex();

    m_treeModel->m_findOpts.m_keys.clear();
    m_treeModel->m_findOpts.m_keys.append(COL::Key);
    m_treeModel->m_findOpts.m_value = idx.model()->index(idx.row(), COL::Key, idx.parent()).data().toString();
    RowFindPrev();
    menuUpdateNeeded();
}

void LogTab::RowFindPrev()
{
    RowFindImpl(-1);
}
void LogTab::RowFindNext()
{
    RowFindImpl(1);
}
void LogTab::RowFindImpl(int offset)
{
    QList<int> lstColumns;
    for(COL col : m_treeModel->m_findOpts.m_keys)
        lstColumns << col;

    int start = ui->treeView->currentIndex().row();
    int i = start;
    int rowCount = m_treeModel->rowCount();
    while (true)
    {
        i += offset;

        if (i >= rowCount)
        {
            i = 0;
        }
        else if (i < 0)
        {
            i = rowCount - 1;
        }

        if (i == start)
        {
            m_bar->ShowMessage(QString("Not found: '%1'").arg(m_treeModel->m_findOpts.m_value), 3000);
            return;
        }

        foreach (int col, lstColumns)
        {
            QModelIndex idx = m_treeModel->index(i, col);
            auto data = m_treeModel->data(idx, Qt::DisplayRole).toString();
            if (m_treeModel->m_findOpts.HasMatch(data))
            {
                ui->treeView->setCurrentIndex(idx);
                m_bar->ShowMessage(QString("Found '%1' on line %2").arg(
                                       m_treeModel->m_findOpts.m_value,
                                       m_treeModel->data(m_treeModel->index(i, 0),Qt::DisplayRole).toString()), 3000);
                return;
            }
        }
    }
}

void LogTab::RowHighlightSelected(COL column)
{
    // Figure out which ones are already filtered
    QSet<QString> alreadyFiltered;
    for (const auto& filter : m_treeModel->GetHighlightFilters()) {
       if ((filter.m_keys.size()==1) && (filter.m_mode==SearchMode::Equals) && (filter.m_keys[0]==column)) {
          alreadyFiltered.insert(filter.m_value);
       }
    }
    // Highlight all selected types
    auto idxList = ui->treeView->selectionModel()->selectedRows();
    for (const auto& idx : idxList) {
       QString value = idx.model()->index(idx.row(), column, idx.parent()).data().toString();
       if (alreadyFiltered.contains(value)) continue;
       SearchOpt newOpt;
       newOpt.m_keys.append(column);
       newOpt.m_mode = SearchMode::Equals;
       newOpt.m_value = value;
       newOpt.m_backgroundColor = m_treeModel->m_colorLibrary.GetNextColor();
       m_treeModel->AddHighlightFilter(newOpt);
       alreadyFiltered.insert(value);
    }
    // Update filtered events & menu bar
    if (m_treeModel->m_highlightOnlyMode) {
       RefilterTreeView();
    }
    menuUpdateNeeded();
}

void LogTab::RowShowGlobalDateTime()
{
    m_treeModel->SetTimeMode(TimeMode::GlobalDateTime);
}

void LogTab::RowShowGlobalTime()
{
    m_treeModel->SetTimeMode(TimeMode::GlobalTime);
}

void LogTab::RowShowTimeDeltas()
{
    QModelIndex idx = ui->treeView->currentIndex();
    auto datetime = idx.model()->index(idx.row(), COL::Time, idx.parent()).data(Qt::UserRole).toDateTime().toMSecsSinceEpoch();
    m_treeModel->ShowDeltas(datetime);
}

void LogTab::InitHeaderMenu()
{
    m_headerMenu = new QMenu(this);
    for (int i = 0; i < m_treeModel->columnCount(); i++)
    {
        QVariant hdata = m_treeModel->headerData(i, Qt::Horizontal, Qt::DisplayRole);
        QAction *action = new QAction(hdata.toString(), m_headerMenu);
        action->setCheckable(true);
        // Make some columns 'unhidable'
        if (i == COL::Value)
        {
            action->setEnabled(false);
        }
        action->setData(i);
        m_headerMenu->addAction(action);
    }

    QAction *actionShowAll = new QAction("Show All", m_headerMenu);
    actionShowAll->setData(-1);

    m_headerMenu->addSeparator();
    m_headerMenu->addAction(actionShowAll);

    connect(m_headerMenu, SIGNAL(triggered(QAction*)),
            this, SLOT(HeaderItemSelected(QAction*)), Qt::UniqueConnection);
}

void LogTab::HeaderRightClicked(const QPoint &pos)
{
    for (int i = 0; i < m_treeModel->columnCount(); i++)
    {
        m_headerMenu->actions()[i]->setChecked(!ui->treeView->isColumnHidden(i));
    }
    m_headerMenu->popup(ui->treeView->viewport()->mapToGlobal(pos));
}

void LogTab::HeaderItemSelected(QAction *action)
{
    int column = action->data().toInt();
    if (column == -1)
    {
        // Show all columns
        for (int i = 0; i < m_treeModel->columnCount(); i++)
        {
            ui->treeView->setColumnHidden(i, false);
        }
    }
    else
    {
        // Toggle the column visibility
        bool checkState = action->isChecked();
        ui->treeView->setColumnHidden(column, !checkState);
    }
}

void LogTab::UpdateStatusBar()
{
    QString status = "";
    if (m_treeModel->HasHighlightFilters())
    {
        status += m_treeModel->m_highlightOnlyMode ? "show only highlighted: {" : "highlighted: {";
        HighlightOptions highlightOpts(m_treeModel->GetHighlightFilters());
        for (int i=0; i< highlightOpts.count(); i++)
        {
            SearchOpt highlightOpt = highlightOpts[i];
            if (i!=0)
            {
                status += ", ";
            }
            status += highlightOpt.m_value;
            if (status.size() > 100)
            {
                status += "...";
                break;
            }
        }
        status += "}";
    }

    m_bar->SetRightLabelText(status);
}

QString LogTab::GetDebugInfo() const
{
    QString tabType;
    QString extra;
    if (m_treeModel->TabType() == TABTYPE::SingleFile)
    {
        tabType = "Single File";
    }
    else if (m_treeModel->TabType() == TABTYPE::Directory)
    {
        tabType = "Directory";
        extra = "Monitoring files:\n  Include:\n";
        for (const QString& file : m_directoryFiles.keys())
        {
            extra += QString("    %1\n").arg(file);
        }
        extra += "  Exclude:\n";
        for (const auto& file : m_excludedFileNames)
        {
            extra += QString("    %1\n").arg(file);
        }
    }
    else if (m_treeModel->TabType() == TABTYPE::ExportedEvents)
    {
        tabType = "Exported Events";
    }

    return QString("Type: %1\nPath: %2\n\n%3").arg(tabType).arg(m_tabPath).arg(extra);
}

void LogTab::RefilterTreeView()
{
    QModelIndex previousIdx = ui->treeView->currentIndex();

    ui->treeView->setUpdatesEnabled(false);

    const int count = m_treeModel->rowCount();
    const QModelIndex idx;

    for (int i = 0; i < count; i++)
    {
        bool hidden = m_treeModel->m_highlightOnlyMode && !m_treeModel->IsHighlightedRow(i);
        ui->treeView->setRowHidden(i, idx, hidden);
    }

    ui->treeView->setUpdatesEnabled(true);
    m_treeModel->layoutChanged();
    //TODO: what if not visible?
    ui->treeView->scrollTo(previousIdx, QAbstractItemView::PositionAtCenter);
}

TreeModel* LogTab::GetTreeModel()
{
   return m_treeModel;
}

QTreeView* LogTab::GetTreeView()
{
   return ui->treeView;
}

void LogTab::CopyFullPath()
{
    QString path = QDir::toNativeSeparators(GetTabPath());
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(path);
}

void LogTab::ShowInFolder()
{
    QString path = GetTabPath();
    if (m_treeModel->TabType() == TABTYPE::SingleFile)
    {
        QFileInfo fi(path);
        path = fi.path();
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}
