#include "searchopt.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>
#include <QRegularExpression>

SearchOpt::SearchOpt() :
    m_value(""),
    m_matchCase(false),
    m_useRegex(false),
    m_backgroundColor(QColor(255, 255, 255))
{
}

bool SearchOpt::HasMatch(QString value)
{
    if (m_useRegex)
    {
        QRegularExpression regex(m_value, m_matchCase ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);
        if(regex.isValid() && regex.match(value).hasMatch())
            return true;
    }
    else
    {
        if (value.contains(m_value, m_matchCase ? Qt::CaseSensitive : Qt::CaseInsensitive))
            return true;
    }
    return false;
}

QMap<COL, QString> SearchOpt::sm_mapColToString{
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

QJsonObject SearchOpt::ToJson()
{
    QJsonObject json;
    json["value"] = m_value;
    QJsonArray keysArray;
    for (COL columnKey : m_keys)
    {
        keysArray.append(sm_mapColToString[columnKey]);
    }
    json["keys"] = keysArray;
    json["matchCase"] = m_matchCase;
    json["useRegex"] = m_useRegex;
    json["backgroundColor"] = QJsonArray{
        m_backgroundColor.red(),
        m_backgroundColor.green(),
        m_backgroundColor.blue()
    };
    return json;
}

QMap<QString, COL> SearchOpt::sm_mapStringToCol{
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

void SearchOpt::FromJson(const QJsonObject& json)
{
    m_value = json["value"].toString();
    QJsonArray keys = json["keys"].toArray();
    m_keys.clear();
    for (auto value : keys)
    {
        m_keys.append(sm_mapStringToCol[value.toString()]);
    }
    m_matchCase = json["matchCase"].toBool();
    m_useRegex = json["useRegex"].toBool();
    QJsonArray backgroundColors = json["backgroundColor"].toArray();
    m_backgroundColor.setRed(backgroundColors[0].toInt());
    m_backgroundColor.setGreen(backgroundColors[1].toInt());
    m_backgroundColor.setBlue(backgroundColors[2].toInt());
}
