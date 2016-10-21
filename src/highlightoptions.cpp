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
    for (const auto& filterJson : json)
    {
        SearchOpt highlightFilter;
        highlightFilter.FromJson(filterJson.toObject());
        this->append(highlightFilter);
    }
}

QVector<QColor> HighlightOptions::GetColors()
{
    QVector<QColor> result;
    for (const auto &searchOpt : *this)
        result.push_back(searchOpt.m_backgroundColor);
    return result;
}
