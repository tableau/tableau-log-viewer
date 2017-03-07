#include "optionsdlg.h"
#include "ui_optionsdlg.h"

#include "options.h"
#include "pathhelper.h"
#include "themeutils.h"

#include <QBitArray>
#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>
#include <QPalette>
#include <QSettings>

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
    QStringList skippedText;
    QBitArray skippedState(ui->listWidget->count());
    for (int i = 0; i < ui->listWidget->count(); i++)
    {
        skippedText.append(ui->listWidget->item(i)->text());
        skippedState[i] = ui->listWidget->item(i)->checkState() == Qt::Checked ? true : false;
    }
    auto serviceEnable = ui->serviceEnable->isChecked();
    auto serviceURL = ui->serviceURLEdit->text();
    auto diffToolPath = ui->diffToolPath->text();
    auto liveEnable = ui->startFutureLiveCapture->isChecked();
    bool captureAllTextFiles = ui->captureAllTextFiles->isChecked();
    auto currFilter = ui->defaultHighlightComboBox->currentText();
    int syntaxHighlightLimit = ui->syntaxHighlightLimitSpinBox->value();
    auto themeName = ui->themeComboBox->currentText();

    QString iniPath = PathHelper::GetConfigIniPath();
    QSettings settings(iniPath, QSettings::IniFormat);
    settings.beginGroup("Options");
    settings.setValue("skippedText", skippedText);
    settings.setValue("skippedState", skippedState);
    settings.setValue("visualizationServiceEnable", serviceEnable);
    settings.setValue("visualizationServiceURL", serviceURL);
    settings.setValue("diffToolPath", diffToolPath);
    settings.setValue("enableLiveCapture", liveEnable);
    settings.setValue("liveCaptureAllTextFiles", captureAllTextFiles);
    settings.setValue("defaultHighlightFilter", currFilter);
    settings.setValue("syntaxHighlightLimit", syntaxHighlightLimit);
    settings.setValue("theme", themeName);
    settings.endGroup();

    Options::GetInstance().ReadSettings();
}

void OptionsDlg::ReadSettings()
{
    Options& options = Options::GetInstance();
    auto skippedText = options.getSkippedText();
    auto skippedState = options.getSkippedState();
    auto serviceEnable = options.getVisualizationServiceEnable();
    auto serviceURL = options.getVisualizationServiceURL();
    auto diffPath =options.getDiffToolPath();
    auto liveEnable = options.getFutureTabsUnderLive();
    bool captureAllTextFiles = options.getCaptureAllTextFiles();
    auto defaultHighlightFilter = options.getDefaultFilterName();
    int syntaxHighlightLimit = options.getSyntaxHighlightLimit();
    auto themeName = options.getTheme();

    for (int i = 0; i < skippedText.length(); i++)
    {
        QListWidgetItem *newItem = new QListWidgetItem(skippedText[i], ui->listWidget);
        newItem->setFlags(newItem->flags() | Qt::ItemIsUserCheckable);
        newItem->setCheckState(skippedState[i] ? Qt::Checked : Qt::Unchecked);
    }
    ui->useEmbedded->setChecked(!serviceEnable ? true : false);
    ui->serviceEnable->setChecked(serviceEnable ? true : false);
    ui->serviceURLEdit->setText(serviceURL);
    ui->serviceURLEdit->setEnabled(serviceEnable);
    ui->diffToolPath->setText(diffPath);
    ui->startFutureLiveCapture->setChecked(liveEnable);
    ui->captureAllTextFiles->setChecked(captureAllTextFiles);
    ui->syntaxHighlightLimitSpinBox->setValue(syntaxHighlightLimit);

    const auto& themeNames = ThemeUtils::GetThemeNames();
    ui->themeComboBox->addItems(themeNames);
    if (!themeNames.contains(themeName))
    {
        themeName = "Native";
    }
    ui->themeComboBox->setCurrentText(themeName);

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
    int filterIdx = ui->defaultHighlightComboBox->findText(defaultHighlightFilter);
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
    ThemeUtils::SwitchTheme(themeName, this->parentWidget());
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
