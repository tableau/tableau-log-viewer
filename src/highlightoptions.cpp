#include "highlightoptions.h"

HighlightOptions::HighlightOptions()
{
}

HighlightOptions::HighlightOptions(const QJsonArray& json)
{
    FromJson(json);
}

QJsonArray HighlightOptions::ToJson()
{
    QJsonArray json;
    for (SearchOpt& highlightFilter : *this)
    {
        json.append(highlightFilter.ToJson());
    }
    return json;
}

void HighlightOptions::FromJson(const QJsonArray& json)
{
    clear();
    for (auto filterJson : json)
    {
        SearchOpt highlightFilter;
        highlightFilter.FromJson(filterJson.toObject());
        this->append(highlightFilter);
    }
}
