#ifndef OPTIONS_H
#define OPTIONS_H

#include "highlightoptions.h"
#include "pathhelper.h"

#include <QBitArray>
#include <QSettings>
#include <QString>
#include <QStringList>

class Options
{
private:
    Options(){ ReadSettings(); }
    Options(const Options&);
    Options& operator= (const Options&);

    QStringList m_skippedText;
    QBitArray m_skippedState;
    bool m_visualizationServiceEnable;
    QString m_visualizationServiceURL;
    QString m_diffToolPath;
    bool m_futureTabsUnderLive;
    QString m_defaultFilterName;
    HighlightOptions m_defaultHighlightOpts;

public:
    static Options& GetInstance()
    {
        static Options sm_options;
        return sm_options;
    }
    static void DeleteInstance();
    void ReadSettings();

    QStringList getSkippedText();
    QBitArray getSkippedState();
    bool getVisualizationServiceEnable();
    QString getVisualizationServiceURL();
    QString getDiffToolPath();
    bool getFutureTabsUnderLive();
    QString getDefaultFilterName();
    HighlightOptions getDefaultHighlightOpts();
    void LoadHighlightFilter(const QString& filterName);
};

#endif // OPTIONS_H
