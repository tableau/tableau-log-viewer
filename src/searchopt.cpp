#include "searchopt.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>
#include <QRegularExpression>

SearchOpt::SearchOpt() :
    m_value(""),
    m_matchCase(false),
    m_mode(SearchMode::Contains),
    m_backgroundColor(QColor(255, 255, 255))
{
}

bool SearchOpt::HasMatch(QString value)
{
    switch (m_mode) {
        case SearchMode::Equals:
             return value.compare(m_value, m_matchCase ? Qt::CaseSensitive : Qt::CaseInsensitive) == 0;
        case SearchMode::Contains:
             return value.contains(m_value, m_matchCase ? Qt::CaseSensitive : Qt::CaseInsensitive);
        case SearchMode::StartsWith:
             return value.startsWith(m_value, m_matchCase ? Qt::CaseSensitive : Qt::CaseInsensitive);
        case SearchMode::EndsWith:
             return value.endsWith(m_value, m_matchCase ? Qt::CaseSensitive : Qt::CaseInsensitive);
        case SearchMode::Regex:
            QRegularExpression regex(m_value, m_matchCase ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);
            return regex.isValid() && regex.match(value).hasMatch();
    }
    return false;
}

static QMap<COL, QString> mapColToString{
    {COL::ID,       "ID"},
    {COL::File,     "File"},
    {COL::Time,     "Time"},
    {COL::PID,      "PID"},
    {COL::TID,      "TID"},
    {COL::Severity, "Severity"},
    {COL::Request,  "Request"},
    {COL::Session,  "Session"},
    {COL::Site,     "Site"},
    {COL::Key,      "Key"},
    {COL::Value,    "Value"}
};

static QMap<SearchMode, QString> mapModeToString{
    {SearchMode::Equals,     "equals"},
    {SearchMode::Contains,   "contains"},
    {SearchMode::StartsWith, "startswith"},
    {SearchMode::EndsWith,   "endswith"},
    {SearchMode::Regex,      "regex"},
};

QJsonObject SearchOpt::ToJson()
{
    QJsonObject json;
    json["value"] = m_value;
    QJsonArray keysArray;
    for (COL columnKey : m_keys)
    {
        keysArray.append(mapColToString[columnKey]);
    }
    json["keys"] = keysArray;
    json["matchCase"] = m_matchCase;
    json["mode"] = mapModeToString[m_mode];
    json["backgroundColor"] = QJsonArray{
        m_backgroundColor.red(),
        m_backgroundColor.green(),
        m_backgroundColor.blue()
    };
    return json;
}

static QMap<QString, COL> mapStringToCol{
    {"ID",       COL::ID},
    {"File",     COL::File},
    {"Time",     COL::Time},
    {"PID",      COL::PID},
    {"TID",      COL::TID},
    {"Severity", COL::Severity},
    {"Request",  COL::Request},
    {"Session",  COL::Session},
    {"Site",     COL::Site},
    {"Key",      COL::Key},
    {"Value",    COL::Value}
};

static QMap<QString, SearchMode> mapStringToMode{
    {"equals",     SearchMode::Equals},
    {"contains",   SearchMode::Contains},
    {"startswith", SearchMode::StartsWith},
    {"endswith",   SearchMode::EndsWith},
    {"regex",      SearchMode::Regex},
};

void SearchOpt::FromJson(const QJsonObject& json)
{
    m_value = json["value"].toString();
    QJsonArray keys = json["keys"].toArray();
    m_keys.clear();
    for (auto value : keys)
    {
        m_keys.append(mapStringToCol[value.toString()]);
    }
    m_matchCase = json["matchCase"].toBool();
    if (json.contains("mode")) {
        m_mode=mapStringToMode[json["mode"].toString()];
    } else if (json.contains("useRegex")) {
        // Logic for parsing legacy format. Remove this code path at some future point in time
        m_mode=json["mode"].toBool() ? SearchMode::Regex : SearchMode::Contains;
    }
    QJsonArray backgroundColors = json["backgroundColor"].toArray();
    m_backgroundColor.setRed(backgroundColors[0].toInt());
    m_backgroundColor.setGreen(backgroundColors[1].toInt());
    m_backgroundColor.setBlue(backgroundColors[2].toInt());
}
