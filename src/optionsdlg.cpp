#include "optionsdlg.h"
#include "ui_optionsdlg.h"

#include "options.h"
#include "pathhelper.h"
#include "qjsonutils.h"
#include "themeutils.h"

#include <QBitArray>
#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>
#include <QPalette>

OptionsDlg::OptionsDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionsDlg)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ReadSettings();
}

OptionsDlg::~OptionsDlg()
{
    delete ui;
}

void OptionsDlg::WriteSettings()
{
    Options& options = Options::GetInstance();

    QStringList skippedText;
    QBitArray skippedState(ui->listWidget->count());
    for (int i = 0; i < ui->listWidget->count(); i++)
    {
        skippedText.append(ui->listWidget->item(i)->text());
        skippedState[i] = ui->listWidget->item(i)->checkState() == Qt::Checked ? true : false;
    }
    options.setSkippedText(skippedText);
    options.setSkippedState(skippedState);

    options.setVisualizationServiceEnable(ui->serviceEnable->isChecked());
    options.setVisualizationServiceURL(ui->serviceURLEdit->text());
    options.setDiffToolPath(ui->diffToolPath->text());
    options.setFutureTabsUnderLive(ui->startFutureLiveCapture->isChecked());
    options.setCaptureAllTextFiles(ui->captureAllTextFiles->isChecked());
    options.setShowArtDataInValue(ui->showArtDataInValue->isChecked());
    options.setShowErrorCodeInValue(ui->showErrorCodeInValue->isChecked());
    options.setDefaultFilterName(ui->defaultHighlightComboBox->currentText());
    options.setSyntaxHighlightLimit(ui->syntaxHighlightLimitSpinBox->value());
    options.setTheme(ui->themeComboBox->currentText());
    options.setNotation(ui->notationComboBox->currentText());

    // Persist the options
    options.WriteSettings();
}

void OptionsDlg::ReadSettings()
{
    Options& options = Options::GetInstance();

    QStringList skippedText = options.getSkippedText();
    QBitArray skippedState = options.getSkippedState();
    for (int i = 0; i < skippedText.length(); i++)
    {
        QListWidgetItem *newItem = new QListWidgetItem(skippedText[i], ui->listWidget);
        newItem->setFlags(newItem->flags() | Qt::ItemIsUserCheckable);
        newItem->setCheckState(skippedState[i] ? Qt::Checked : Qt::Unchecked);
    }

    bool serviceEnable = options.getVisualizationServiceEnable();
    ui->useEmbedded->setChecked(!serviceEnable);
    ui->serviceEnable->setChecked(serviceEnable);
    ui->serviceURLEdit->setEnabled(serviceEnable);

    ui->serviceURLEdit->setText(options.getVisualizationServiceURL());
    ui->diffToolPath->setText(options.getDiffToolPath());
    ui->startFutureLiveCapture->setChecked(options.getFutureTabsUnderLive());
    ui->captureAllTextFiles->setChecked(options.getCaptureAllTextFiles());
    ui->showArtDataInValue->setChecked(options.getShowArtDataInValue());
    ui->showErrorCodeInValue->setChecked(options.getShowErrorCodeInValue());
    ui->syntaxHighlightLimitSpinBox->setValue(options.getSyntaxHighlightLimit());

    const auto& themeNames = ThemeUtils::GetThemeNames();
    ui->themeComboBox->addItems(themeNames);
    QString themeName = options.getTheme();
    if (!themeNames.contains(themeName))
    {
        themeName = "Native";
    }
    ui->themeComboBox->setCurrentText(themeName);

    const auto& notationNames = QJsonUtils::GetNotationNames();
    ui->notationComboBox->addItems(notationNames);
    QString notationName = options.getNotation();
    if (!notationNames.contains(notationName))
    {
        notationName = "YAML";
    }
    ui->notationComboBox->setCurrentText(notationName);

    // load all saved filters for default filters
    ui->defaultHighlightComboBox->addItem(QString("None"));
    QDir loadDir(PathHelper::GetFiltersConfigPath());
    if (loadDir.exists())
    {
        QString filterExtension = QStringLiteral("*.json");
        QStringList filters;
        for (QString entry : loadDir.entryList(QStringList(filterExtension), QDir::Filter::Files))
        {
            // Add an item with the filter file. Take out the ".json" at the end
            filters.push_back(entry.left(entry.length() - filterExtension.length() + 1));
        }
        ui->defaultHighlightComboBox->addItems(filters);
    }
    int filterIdx = ui->defaultHighlightComboBox->findText(options.getDefaultFilterName());
    ui->defaultHighlightComboBox->setCurrentIndex(filterIdx);
}

void OptionsDlg::on_listWidget_itemSelectionChanged()
{
    qDebug() << "ui->listWidget->currentRow()" << ui->listWidget->currentRow();
    ui->btnRemove->setEnabled(ui->listWidget->currentRow() != -1);
}

void OptionsDlg::on_btnAdd_clicked()
{
    bool ok;
    QString text = QInputDialog::getText(this, "Add", "Add new event to skip on load:", QLineEdit::Normal, "", &ok);
    if (ok)
    {
        QListWidgetItem *newItem = new QListWidgetItem(text, ui->listWidget);
        newItem->setFlags(newItem->flags() | Qt::ItemIsUserCheckable);
        newItem->setCheckState(Qt::Checked);
    }
}

void OptionsDlg::on_btnRemove_clicked()
{
    qDebug() << "removing" << ui->listWidget->currentItem();
    delete ui->listWidget->takeItem(ui->listWidget->currentRow());
}

void OptionsDlg::on_chooseDiffTool_clicked()
{
    auto fileName = QFileDialog::getOpenFileName(this, "Select Diff Tool", "/home", "");
    if(!fileName.isNull())
        ui->diffToolPath->setText(fileName);
}

void OptionsDlg::on_useEmbedded_clicked()
{
    ui->serviceURLEdit->setDisabled(true);
}

void OptionsDlg::on_serviceEnable_clicked()
{
    ui->serviceURLEdit->setEnabled(true);
}

void OptionsDlg::on_themeComboBox_currentTextChanged(const QString &themeName)
{
    // Only switch the theme after the dialog window is fully loaded (issue #59)
    if (this->isVisible())
    {
        ThemeUtils::SwitchTheme(themeName, this->parentWidget());
    }
}

void OptionsDlg::on_OptionsDlg_accepted()
{
    WriteSettings();
}

void OptionsDlg::on_OptionsDlg_rejected()
{
    // Revert the theme back if the user cancels the dialog
    Options& options = Options::GetInstance();
    auto themeNameSettings = options.getTheme();
    if (!ThemeUtils::GetThemeNames().contains(themeNameSettings))
    {
        themeNameSettings = "Native";
    }

    auto themeNameUi = ui->themeComboBox->currentText();
    // Only apply a theme change if necessary
    if (themeNameSettings != themeNameUi)
    {
        ThemeUtils::SwitchTheme(themeNameSettings, this->parentWidget());
    }
}
