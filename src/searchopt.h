#ifndef SEARCHOPT_H
#define SEARCHOPT_H

#include "column.h"

#include <QColor>
#include <QJsonObject>
#include <QString>
#include <QVector>

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
    bool m_useRegex;
    QColor m_backgroundColor;

private:
    static QMap<COL, QString> sm_mapColToString;
    static QMap<QString, COL> sm_mapStringToCol;
};

#endif // SEARCHOPT_H
