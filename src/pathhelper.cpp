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
        return GetConfigPath() + QDir::separator() + QStringLiteral("filters");
    }

    QString GetConfigIniPath()
    {
        return GetConfigPath() + QDir::separator() + QStringLiteral("tlv.ini");
    }

    QString GetDocumentsPath()
    {
        return QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    }

    QString GetTableauRepositoryPath(bool isBeta)
    {
        return GetDocumentsPath() + QDir::separator() +
            (isBeta ? QStringLiteral("My Tableau Repository (beta)") : QStringLiteral("My Tableau Repository"));
    }

    QString GetTableauLogFolderPath(bool isBeta)
    {
        return GetTableauRepositoryPath(isBeta) + QDir::separator() + QStringLiteral("Logs");
    }

    QString GetTableauLogFilePath(bool isBeta)
    {
        return GetTableauLogFolderPath(isBeta) + QDir::separator() + QStringLiteral("log.txt");
    }
}
