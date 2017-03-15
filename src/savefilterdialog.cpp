#include "savefilterdialog.h"
#include "ui_savefilterdialog.h"
#include "options.h"

#include <QDebug>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QMessageBox>
#include <QStandardPaths>

SaveFilterDialog::SaveFilterDialog(QWidget *parent, const HighlightOptions& filters) :
    QDialog(parent),
    ui(new Ui::SaveFilterDialog),
    m_filters(filters)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui->setupUi(this);
    ui->lineEdit->setFocus();
}

SaveFilterDialog::~SaveFilterDialog()
{
    delete ui;
}

void SaveFilterDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    auto role = ui->buttonBox->standardButton(button);
    if (role == QDialogButtonBox::Save)
    {
        QString filename = ui->lineEdit->text();

        bool success = SaveFilter(filename);
        if (success)
        {
            Options& options = Options::GetInstance();
            if (options.getDefaultFilterName() == filename)
            {
                options.LoadHighlightFilter(filename);
            }
            accept();
        }
    }
}

bool SaveFilterDialog::SaveFilter(const QString& filename)
{
    qDebug() << "attempting to save...";

    // Validate filename
    QRegularExpression regex("^[a-z0-9.\\-_ ]+$", QRegularExpression::CaseInsensitiveOption);
    if (!regex.match(filename).hasMatch())
    {
        QMessageBox validationWarning(
            QMessageBox::Icon::Warning,
            QStringLiteral("Invalid name"),
            QStringLiteral("Only alphanumeric characters, underscores, dashes, periods, and spaces are allowed."));
        validationWarning.exec();
        return false;
    }

    // Get AppConfig folder, create it if it doesn't exists
    QDir saveFolder(QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation)[0] + "/filters");
    if (!saveFolder.exists())
    {
        bool folderSuccess = saveFolder.mkpath(".");
        if (!folderSuccess)
        {
            QMessageBox configFolderWarning(
                QMessageBox::Icon::Warning,
                QStringLiteral("Error saving configuration"),
                QStringLiteral("Config folder didn't exist and couldn't be created."));
            configFolderWarning.exec();
            return false;
        }
    }

    // Open file in write mode
    QFile saveFile(saveFolder.filePath(filename + ".json"));
    qDebug() << saveFile.fileName();
    if (!saveFile.open(QIODevice::WriteOnly))
    {
        QMessageBox configFileWarning(
            QMessageBox::Icon::Warning,
            QStringLiteral("Error saving configuration"),
            QStringLiteral("Config file couldn't be opened."));
        configFileWarning.exec();
        return false;
    }

    QJsonDocument filtersDoc(m_filters.ToJson());
    saveFile.write(filtersDoc.toJson());
    saveFile.close();
    return true;
}
