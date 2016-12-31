#pragma once

#include "highlightoptions.h"

#include <QBitArray>

class Options
{
private:
    Options(){ ReadSettings(); }
    Options(const Options&) = delete;
    Options& operator= (const Options&) = delete;

    QStringList m_skippedText;
    QBitArray m_skippedState;
    bool m_visualizationServiceEnable;
    QString m_visualizationServiceURL;
    QString m_diffToolPath;
    bool m_futureTabsUnderLive;
    bool m_captureAllTextFiles;
    QString m_defaultFilterName;
    HighlightOptions m_defaultHighlightOpts;
    int m_syntaxHighlightLimit;
    QString m_backgroundColor;
    QString m_theme;

public:
    static Options& GetInstance()
    {
        static Options options;
        return options;
    }
    void ReadSettings();
    void LoadHighlightFilter(const QString& filterName);

    QStringList getSkippedText();
    QBitArray getSkippedState();
    bool getVisualizationServiceEnable();
    QString getVisualizationServiceURL();
    QString getDiffToolPath();
    bool getFutureTabsUnderLive();
    bool getCaptureAllTextFiles();
    QString getDefaultFilterName();
    HighlightOptions getDefaultHighlightOpts();
    int getSyntaxHighlightLimit() const;
    QString getBackgroundColor() const;

    const static QString DefaultColorWhite;
    const static QString DefaultColorGray;
    QString getTheme();
};
