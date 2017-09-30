#ifndef SEARCHOPT_H
#define SEARCHOPT_H

#include "column.h"

#include <QColor>
#include <QJsonObject>
#include <QString>
#include <QVector>

enum SearchMode : short {
    Equals,
    Contains,
    StartsWith,
    EndsWith,
    Regex
};

class SearchOpt
{
public:
    SearchOpt();
    bool HasMatch(QString value);
    QJsonObject ToJson();
    void FromJson(const QJsonObject& json);

    QString m_value;
    QVector<COL> m_keys;
    bool m_matchCase;
    SearchMode m_mode;
    QColor m_backgroundColor;
};

#endif // SEARCHOPT_H
