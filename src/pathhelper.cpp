#include "pathhelper.h"

#include <QDir>
#include <QStandardPaths>

namespace PathHelper
{
    QString GetConfigPath()
    {
        return QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation)[0];
    }

    QString GetFiltersConfigPath()
    {
        return GetConfigPath() + "/" + QStringLiteral("filters");
    }

    QString GetConfigIniPath()
    {
        return GetConfigPath() + "/" + QStringLiteral("tlv.ini");
    }

    QString GetDocumentsPath()
    {
        return QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    }

    QString GetTableauRepositoryPath(bool isBeta)
    {
        return GetDocumentsPath() + "/" +
            (isBeta ? QStringLiteral("My Tableau Repository (Beta)") : QStringLiteral("My Tableau Repository"));
    }

    QString GetTableauLogFolderPath(bool isBeta)
    {
        return GetTableauRepositoryPath(isBeta) + "/" + QStringLiteral("Logs");
    }

    QString GetTableauLogFilePath(bool isBeta)
    {
        return GetTableauLogFolderPath(isBeta) + "/" + QStringLiteral("log.txt");
    }
}
