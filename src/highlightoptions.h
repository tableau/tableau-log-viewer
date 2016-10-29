#ifndef HIGHLIGHTOPTIONS_H
#define HIGHLIGHTOPTIONS_H

#include "searchopt.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QVector>

class HighlightOptions : public QVector<SearchOpt>
{
public:
    HighlightOptions();
    HighlightOptions(const QJsonArray& json);
    QJsonArray ToJson();
    void FromJson(const QJsonArray& json);
    QVector<QColor> GetColors();
};

#endif // HIGHLIGHTOPTIONS_H
