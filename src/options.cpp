#include "options.h"
#include "pathhelper.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QSettings>
#include <QSysInfo>

void Options::ReadSettings()
{
    QString iniPath = PathHelper::GetConfigIniPath();
    QSettings settings(iniPath, QSettings::IniFormat);
    settings.beginGroup("Options");

    QStringList defaultSkip = {"dll-version-info", "ds-interpret-metadata"};
    m_skippedText = settings.value("skippedText", defaultSkip).toStringList();
    m_skippedState = settings.value("skippedState", QBitArray(m_skippedText.length(), true)).toBitArray();
    m_visualizationServiceEnable = settings.value("visualizationServiceEnable", false).toBool();
    m_visualizationServiceURL = settings.value("visualizationServiceURL", QString("")).toString();
    auto defaultDiffToolPath = QSysInfo::productType() == "windows" ? QString("C:/Program Files (x86)/Beyond Compare 4/BCompare.exe") : QString("/usr/local/bin/bcomp");
    m_diffToolPath = settings.value("diffToolPath", defaultDiffToolPath).toString();
    m_futureTabsUnderLive = settings.value("enableLiveCapture").toBool();
    m_defaultFilterName = settings.value("defaultHighlightFilter", "None").toString();
    m_captureAllTextFiles = settings.value("liveCaptureAllTextFiles", true).toBool();
    m_syntaxHighlightLimit = settings.value("syntaxHighlightLimit", 15000).toInt();
    m_theme = settings.value("theme", "Native").toString();

    settings.endGroup();

    if(m_defaultFilterName == "None")
    {
        m_defaultHighlightOpts.clear();
    }
    else
    {
        LoadHighlightFilter(m_defaultFilterName);
    }
}

QStringList Options::getSkippedText()
{
    return m_skippedText;
}

QBitArray Options::getSkippedState()
{
    return m_skippedState;
}

bool Options::getVisualizationServiceEnable()
{
    return m_visualizationServiceEnable;
}

QString Options::getVisualizationServiceURL()
{
    return m_visualizationServiceURL;
}

QString Options::getDiffToolPath()
{
    return m_diffToolPath;
}

bool Options::getFutureTabsUnderLive()
{
    return m_futureTabsUnderLive;
}

bool Options::getCaptureAllTextFiles()
{
    return m_captureAllTextFiles;
}

QString Options::getDefaultFilterName()
{
    return m_defaultFilterName;
}

HighlightOptions Options::getDefaultHighlightOpts()
{
    return m_defaultHighlightOpts;
}

void Options::LoadHighlightFilter(const QString& filterName)
{
    QDir loadDir(PathHelper::GetFiltersConfigPath());
    QFile loadFile(loadDir.filePath(filterName + ".json"));
    if (!loadFile.open(QIODevice::ReadOnly))
    {
        qWarning("Couldn't open file.");
        return;
    }
    QByteArray filterData = loadFile.readAll();
    QJsonDocument filtersDoc(QJsonDocument::fromJson(filterData));
    m_defaultHighlightOpts.FromJson(filtersDoc.array());
    // Currently this does not apply to already open tabs, but I think that's fine.
}

int Options::getSyntaxHighlightLimit() const
{
    return m_syntaxHighlightLimit;
}

QString Options::getTheme()
{
    return m_theme;
}
