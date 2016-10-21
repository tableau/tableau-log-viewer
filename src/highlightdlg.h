#ifndef HIGHLIGHTDLG_H
#define HIGHLIGHTDLG_H

#include "colorlibrary.h"
#include "filtertab.h"
#include "highlightoptions.h"
#include "searchopt.h"

#include <QDialog>
#include <QTabWidget>

namespace Ui {
class HighlightDlg;
}

class HighlightDlg : public QDialog
{
    Q_OBJECT

public:
    HighlightDlg(QWidget *parent, HighlightOptions highlightOpts, const ColorLibrary& colorLibrary);
    ~HighlightDlg();
    void keyPressEvent(QKeyEvent * k);

    HighlightOptions m_highlightOpts;
    ColorLibrary m_colorLibrary;

private slots:
    void on_tabWidget_tabCloseRequested(int index);
    void on_tabWidget_currentChanged(int index);
    void TextChanged(const QString & text);

private:
    void BuildTabs(const HighlightOptions& highlightOpts);
    void AddNewTab();
    void ConstructTab(SearchOpt options);
    void CloseCurrentTab();
    void CloseTab(int index);
    void accepted();

    Ui::HighlightDlg *ui;
    const QString m_filterTabValueLabel = "Highlight &log event values that contains substring:";
    const int m_tabLabelSize = 20;
};

#endif // HIGHLIGHTDLG_H
