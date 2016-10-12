#ifndef PATHHELPER_H
#define PATHHELPER_H

#include <QString>

namespace PathHelper
{
    QString GetConfigPath();
    QString GetConfigIniPath();
    QString GetFiltersConfigPath();
    QString GetDocumentsPath();
    QString GetTableauRepositoryPath(bool isBeta = false);
    QString GetTableauLogFolderPath(bool isBeta = false);
    QString GetTableauLogFilePath(bool isBeta = false);
}

#endif // PATHHELPER_H
