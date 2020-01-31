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
    void on_prevButton_clicked();
    void on_nextButton_clicked();

signals:
    void next();
    void prev();

private:
    void ConstructTab(SearchOpt option);
    void UpdateFindOptions();

    Ui::FindDlg *ui;
};

#endif // FINDDLG_H
