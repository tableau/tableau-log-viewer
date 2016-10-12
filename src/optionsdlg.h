#ifndef OPTIONSDLG_H
#define OPTIONSDLG_H

#include <QDialog>
#include <QStringList>

namespace Ui {
class OptionsDlg;
}

class OptionsDlg : public QDialog
{
    Q_OBJECT

public:
    explicit OptionsDlg(QWidget *parent = 0);
    ~OptionsDlg();

private slots:
    //Slots use underscores as per QT's automatic connection syntax
    void on_btnAdd_clicked();
    void on_OptionsDlg_accepted();
    void on_listWidget_itemSelectionChanged();
    void on_btnRemove_clicked();
    void on_chooseDiffTool_clicked();
    void on_useEmbedded_clicked();
    void on_serviceEnable_clicked();

private:
    void WriteSettings();
    void ReadSettings();

    Ui::OptionsDlg *ui;
};

#endif // OPTIONSDLG_H
