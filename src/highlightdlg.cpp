#include "highlightdlg.h"
#include "ui_highlightdlg.h"
#include "column.h"

#include <QDebug>
#include <QString>
#include <QKeyEvent>
#include <QTabWidget>
#include <QWidget>
#include <QGroupBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QMap>

HighlightDlg::HighlightDlg(QWidget *parent, HighlightOptions highlightOpts, const ColorLibrary& colorLibrary) :
    QDialog(parent),
    ui(new Ui::HighlightDlg)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // Remove close button of the Help tab.
    ui->tabWidget->tabBar()->tabButton(0, QTabBar::RightSide)->resize(0, 0);

    m_colorLibrary = colorLibrary;
    connect(this, &QDialog::accepted, this, &HighlightDlg::accepted);
    BuildTabs(highlightOpts);
}

void HighlightDlg::BuildTabs(const HighlightOptions& highlightOpts)
{
    if(highlightOpts.count() == 0)
    {
        AddNewTab();
    }
    for(auto option : highlightOpts)
    {
        ConstructTab(option);
    }
}

void HighlightDlg::keyPressEvent(QKeyEvent * k)
{
    if((k->key() == Qt::Key_T || k->key() == Qt::Key_N) && (QApplication::keyboardModifiers() & Qt::ControlModifier))
    {
        AddNewTab();
    }
    else if((k->key() == Qt::Key_W || k->key() == Qt::Key_F4) && (QApplication::keyboardModifiers() & Qt::ControlModifier))
    {
        CloseCurrentTab();
    }
    else if(k->key() == Qt::Key_Enter || k->key() == Qt::Key_Return)
    {
        accept();
    }
    else
    {
        QDialog::keyPressEvent(k);
    }
}

void HighlightDlg::ConstructTab(SearchOpt option)
{
    // Populate the tab based on the given filter
    auto filterTab = new FilterTab(nullptr, m_filterTabValueLabel, option.m_backgroundColor);
    ui->tabWidget->addTab(filterTab, option.m_value.left(m_tabLabelSize));
    ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(filterTab));
    filterTab->SetSearchOptions(option);
    connect(filterTab, &FilterTab::filterValueChanged, this, &HighlightDlg::TextChanged);
}

void HighlightDlg::AddNewTab()
{
    auto newFilter = new FilterTab(nullptr, m_filterTabValueLabel);
    ui->tabWidget->addTab(newFilter, "   ");
    ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(newFilter));
    newFilter->SetBackgroundColor(m_colorLibrary.GetNextColor());

    connect(newFilter, &FilterTab::filterValueChanged, this, &HighlightDlg::TextChanged);
}

void HighlightDlg::TextChanged(const QString & text)
{
    ui->tabWidget->setTabText(ui->tabWidget->currentIndex(), text.left(m_tabLabelSize));
}

void HighlightDlg::CloseCurrentTab()
{
    CloseTab(ui->tabWidget->currentIndex());
}

void HighlightDlg::CloseTab(int index)
{
    auto filterTab = dynamic_cast<FilterTab *>(ui->tabWidget->widget(index));
    if (!filterTab)
    {
        return;
    }
    ui->tabWidget->removeTab(index);
}

void HighlightDlg::on_tabWidget_currentChanged(int index)
{
    auto filterTab = dynamic_cast<FilterTab *>(ui->tabWidget->widget(index));
    if (!filterTab)
    {
        return;
    }
    filterTab->SetFocusOnFilterLine();
}

void HighlightDlg::on_tabWidget_tabCloseRequested(int index)
{
    CloseTab(index);
}

void HighlightDlg::accepted()
{
    m_highlightOpts.clear();
    for (int i = 0; i < ui->tabWidget->count(); i++)
    {
        auto filterTab = dynamic_cast<FilterTab *>(ui->tabWidget->widget(i));
        if (!filterTab)
            continue;
        SearchOpt newSearchOpt = filterTab->GetSearchOptions();
        if(!newSearchOpt.m_value.isEmpty() && !newSearchOpt.m_keys.isEmpty())
        {
            m_highlightOpts.append(newSearchOpt);
        }
    }
}

HighlightDlg::~HighlightDlg()
{
    delete ui;
}
