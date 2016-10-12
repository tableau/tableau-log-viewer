#include "finddlg.h"
#include "ui_finddlg.h"

#include <QCheckBox>
#include <QGroupBox>
#include <QLineEdit>

FindDlg::FindDlg(QWidget *parent, SearchOpt findOpts) :
    QDialog(parent),
    ui(new Ui::FindDlg)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    if(findOpts.m_keys.count() > 0 && findOpts.m_value != "")
    {
        ConstructTab(findOpts);
    }
    else
    {
        SearchOpt blankOpt;
        blankOpt.m_keys.append(COL::Key);
        blankOpt.m_keys.append(COL::Value);
        ConstructTab(blankOpt);
    }
    connect(this, &QDialog::accepted, this, &FindDlg::accepted);

}

FindDlg::~FindDlg()
{
    delete ui;
}

void FindDlg::ConstructTab(SearchOpt options)
{
    // Populate the tab based on the given filter
    ui->filterTab->SetTitle("Find log event values that contains substring:");
    ui->filterTab->SetSearchOptions(options);
    ui->filterTab->SetFocusOnFilterLine();
    ui->filterTab->HideColorWidget();
}

void FindDlg::accepted()
{
    SearchOpt newSearchOpt = ui->filterTab->GetSearchOptions();
    if(!newSearchOpt.m_value.isEmpty() && !newSearchOpt.m_keys.isEmpty())
    {
        m_findOpts = newSearchOpt;
    }
}
