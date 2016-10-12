#ifndef FILTERTAB_H
#define FILTERTAB_H

#include "column.h"
#include "searchopt.h"

#include <QCheckBox>
#include <QMap>
#include <QString>
#include <QWidget>

namespace Ui {
class FilterTab;
}

class FilterTab : public QWidget
{
    Q_OBJECT

public:
    explicit FilterTab(QWidget *parent = 0, QString valueText = "", QColor backgroundColor = QColor(255, 255, 255));
    ~FilterTab();
    void SetTitle(const QString& title);
    void SetFocusOnFilterLine();
    void SetBackgroundColor(const QColor& backgroundColor);
    void HideColorWidget();
    void SetSearchOptions(const SearchOpt& options);
    SearchOpt GetSearchOptions();

private slots:
    void keyPressEvent(QKeyEvent * k);
    void on_filterLineEdit_textChanged(const QString &text);
    void on_buttonChangeColor_clicked();

signals:
    void filterValueChanged(const QString& text);

private:
    Ui::FilterTab *ui;
    QColor m_backgroundColor;
    std::map<COL, QCheckBox*> m_mapColToBox;
};

#endif // FILTERTAB_H
