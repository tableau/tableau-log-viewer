#ifndef FINDDLG_H
#define FINDDLG_H

#include "filtertab.h"
#include "searchopt.h"

#include <QDialog>

namespace Ui {
class FindDlg;
}

class FindDlg : public QDialog
{
    Q_OBJECT

public:
    FindDlg(QWidget *parent, SearchOpt findOpts);
    ~FindDlg();

    SearchOpt m_findOpts;

private slots:
    void accepted();

private:
    void ConstructTab(SearchOpt option);

    Ui::FindDlg *ui;
};

#endif // FINDDLG_H
