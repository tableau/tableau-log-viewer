#ifndef SAVEFILTERDIALOG_H
#define SAVEFILTERDIALOG_H

#include "highlightoptions.h"

#include <QAbstractButton>
#include <QDialog>

namespace Ui {
class SaveFilterDialog;
}

class SaveFilterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SaveFilterDialog(QWidget *parent, const HighlightOptions& filters);
    ~SaveFilterDialog();

private slots:
    void on_buttonBox_clicked(QAbstractButton *button);

private:
    bool SaveFilter(const QString& filename);

    Ui::SaveFilterDialog *ui;
    HighlightOptions m_filters;
};

#endif // SAVEFILTERDIALOG_H
