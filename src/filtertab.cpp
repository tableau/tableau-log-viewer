#include "filtertab.h"
#include "ui_filtertab.h"

#include "highlightdlg.h"

#include <QColorDialog>
#include <QGroupBox>

FilterTab::FilterTab(QWidget *parent, QString valueText, QColor backgroundColor) :
    QWidget(parent),
    ui(new Ui::FilterTab)
{
    ui->setupUi(this);
    ui->checkBoxKey->setChecked(true);
    ui->checkBoxValue->setChecked(true);
    SetTitle(valueText);
    SetBackgroundColor(backgroundColor);

    // mapping of COL and checkBox widgets
    m_mapColToBox = std::map<COL, QCheckBox*> {
        {COL::Severity,   ui->checkBoxSeverity},
        {COL::TID,        ui->checkBoxTID},
        {COL::PID,        ui->checkBoxPID},
        {COL::Request,    ui->checkBoxRequest},
        {COL::Session,    ui->checkBoxSession},
        {COL::Site,       ui->checkBoxSite},
        {COL::User,       ui->checkBoxUser},
        {COL::Key,        ui->checkBoxKey},
        {COL::Value,      ui->checkBoxValue},
        {COL::File,       ui->checkBoxFile}
    };
}

FilterTab::~FilterTab()
{
    delete ui;
}

void FilterTab::SetTitle(const QString& title)
{
    ui->groupBoxValue->setTitle(title);
}

void FilterTab::SetFocusOnFilterLine()
{
    ui->filterLineEdit->setFocus();
}

void FilterTab::SetBackgroundColor(const QColor& backgroundColor)
{
    m_backgroundColor = backgroundColor;
    ui->colorDisplay->setStyleSheet("background-color: " + m_backgroundColor.name());
}

void FilterTab::HideColorWidget()
{
    ui->colorWidget->hide();
}

void FilterTab::SetSearchOptions(const SearchOpt& options)
{
    ui->filterLineEdit->setText(options.m_value);
    ui->checkBoxRegEx->setChecked(options.m_useRegex);
    ui->checkBoxCase->setChecked(options.m_matchCase);
    for (auto& colToBox : m_mapColToBox)
    {
        colToBox.second->setChecked(options.m_keys.contains(colToBox.first));
    }
}

SearchOpt FilterTab::GetSearchOptions()
{
    SearchOpt options;
    options.m_value = ui->filterLineEdit->text();
    options.m_useRegex = ui->checkBoxRegEx->isChecked();
    options.m_matchCase = ui->checkBoxCase->isChecked();

    for (const auto& colToBox : m_mapColToBox)
    {
        if (colToBox.second->isChecked())
        {
            options.m_keys.append(colToBox.first);
        }
    }
    options.m_backgroundColor = m_backgroundColor;
    return options;
}

void FilterTab::keyPressEvent(QKeyEvent * k)
{
    QTabWidget *parentTab = (QTabWidget *)this->parentWidget();
    HighlightDlg *parentDlg = (HighlightDlg *)parentTab->parentWidget();
    parentDlg->keyPressEvent(k);
}

void FilterTab::on_filterLineEdit_textChanged(const QString &text)
{
    filterValueChanged(text);
}

void FilterTab::on_buttonChangeColor_clicked()
{
    QColor pickedColor = QColorDialog::getColor(/* initial = */m_backgroundColor, this);
    if (pickedColor.isValid())
    {
        SetBackgroundColor(pickedColor);
    }
}
