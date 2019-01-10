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
    m_showArtDataInValue = settings.value("showArtDataInValue", false).toBool();
    m_showErrorCodeInValue = settings.value("showErrorCodeInValue", false).toBool();
    m_syntaxHighlightLimit = settings.value("syntaxHighlightLimit", 15000).toInt();
    m_theme = settings.value("theme", "Native").toString();
    m_notation = settings.value("notation", "YAML").toString();

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

void Options::WriteSettings()
{
    QString iniPath = PathHelper::GetConfigIniPath();
    QSettings settings(iniPath, QSettings::IniFormat);

    settings.beginGroup("Options");
    settings.setValue("skippedText", m_skippedText);
    settings.setValue("skippedState", m_skippedState);
    settings.setValue("visualizationServiceEnable", m_visualizationServiceEnable);
    settings.setValue("visualizationServiceURL", m_visualizationServiceURL);
    settings.setValue("diffToolPath", m_diffToolPath);
    settings.setValue("enableLiveCapture", m_futureTabsUnderLive);
    settings.setValue("liveCaptureAllTextFiles", m_captureAllTextFiles);
    settings.setValue("showArtDataInValue", m_showArtDataInValue);
    settings.setValue("showErrorCodeInValue", m_showErrorCodeInValue);
    settings.setValue("defaultHighlightFilter", m_defaultFilterName);
    settings.setValue("syntaxHighlightLimit", m_syntaxHighlightLimit);
    settings.setValue("theme", m_theme);
    settings.setValue("notation", m_notation);
    settings.endGroup();
}

QStringList Options::getSkippedText() const
{
    return m_skippedText;
}

void Options::setSkippedText(const QStringList &skippedText)
{
    m_skippedText = skippedText;
}

QBitArray Options::getSkippedState() const
{
    return m_skippedState;
}

void Options::setSkippedState(const QBitArray &skippedState)
{
    m_skippedState = skippedState;
}

bool Options::getVisualizationServiceEnable() const
{
    return m_visualizationServiceEnable;
}

void Options::setVisualizationServiceEnable(const bool visualizationServiceEnable)
{
    m_visualizationServiceEnable = visualizationServiceEnable;
}

QString Options::getVisualizationServiceURL() const
{
    return m_visualizationServiceURL;
}

void Options::setVisualizationServiceURL(const QString &visualizationServiceURL)
{
    m_visualizationServiceURL = visualizationServiceURL;
}

QString Options::getDiffToolPath() const
{
    return m_diffToolPath;
}

void Options::setDiffToolPath(const QString &diffToolPath)
{
    m_diffToolPath = diffToolPath;
}

bool Options::getFutureTabsUnderLive() const
{
    return m_futureTabsUnderLive;
}

void Options::setFutureTabsUnderLive(const bool futureTabsUnderLive)
{
    m_futureTabsUnderLive = futureTabsUnderLive;
}

bool Options::getShowArtDataInValue() const
{
    return m_showArtDataInValue;
}

void Options::setShowArtDataInValue(const bool showArtDataInValue)
{
    m_showArtDataInValue = showArtDataInValue;
}

bool Options::getShowErrorCodeInValue() const
{
    return m_showErrorCodeInValue;
}

void Options::setShowErrorCodeInValue(const bool showErrorCodeInValue)
{
    m_showErrorCodeInValue = showErrorCodeInValue;
}

bool Options::getCaptureAllTextFiles() const
{
    return m_captureAllTextFiles;
}

void Options::setCaptureAllTextFiles(const bool captureAllTextFiles)
{
    m_captureAllTextFiles = captureAllTextFiles;
}

QString Options::getDefaultFilterName() const
{
    return m_defaultFilterName;
}

void Options::setDefaultFilterName(const QString &defaultFilterName)
{
    m_defaultFilterName = defaultFilterName;
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

void Options::setSyntaxHighlightLimit(const int syntaxHighlightLimit)
{
    m_syntaxHighlightLimit = syntaxHighlightLimit;
}

QString Options::getTheme() const
{
    return m_theme;
}

void Options::setTheme(const QString &theme)
{
    m_theme = theme;
}

QString Options::getNotation() const
{
    return m_notation;
}

void Options::setNotation(const QString &notation)
{
    m_notation = notation;
}
