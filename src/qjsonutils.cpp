#include "qjsonutils.h"

#include <QString>
#include <QStringBuilder>
#include <QVector>
#include <QJsonArray>
#include <QJsonObject>

namespace
{
    void GetFlatNotation(const QString& key, const QJsonValue& value, QVector<QString>& stringList);
    const QString KeyValueString(const QString& key, const QString& value);
}

QString QJsonUtils::Format(const QJsonValue& jsonValue, Notation format, LineFormat lineFormat)
{
    bool isSingleLine = (lineFormat == LineFormat::SingleLine);
    if (format == Notation::Flat)
    {
        QVector<QString> stringList;
        GetFlatNotation("", jsonValue, stringList);
        if (stringList.size() == 0)
            return "";
        QString lineBreak = isSingleLine ? "; " : "\n";
        QString result;
        for (const QString& str : stringList)
        {
            result = result % lineBreak % str;
        }
        result = result.remove(0, lineBreak.size());
        if (isSingleLine)
            result = result.replace("\n", " ");
        return result;
    }
    else {
        return "NOT IMPLEMENTED";
    }
}

namespace
{
    void GetFlatNotation(const QString& key, const QJsonValue& value, QVector<QString>& stringList)
    {
        if (value.isDouble())
        {
            // There are no ints in JSON, only doubles
            // Check if the double value is just an integer and format accordingly
            double intpart;
            const int decimals = (modf(value.toDouble(), &intpart) == 0) ? 0 : 3;
            stringList.append(KeyValueString(key, QString::number(value.toDouble(), 'f', decimals)));
        }
        else if (value.isObject())
        {
            QJsonObject childObj = value.toObject();
            for (QJsonObject::ConstIterator iter = childObj.constBegin(); iter != childObj.constEnd(); ++iter)
            {
                GetFlatNotation(iter.key(), iter.value(), stringList);
            }
        }
        else if (value.isArray())
        {
            QJsonArray array = value.toArray();
            int itemIndex = 1;
            stringList.append(KeyValueString(key, ""));
            for (const QJsonValue& itemValue : array)
            {
                auto itemKey = QString("%1-%2").arg(key).arg(itemIndex++);
                GetFlatNotation(itemKey, itemValue, stringList);
            }
        }
        else if (value.isBool())
        {
            stringList.append(KeyValueString(key, QString(value.toBool() ? "true" : "false")));
        }
        else if (value.isNull())
        {
            stringList.append(KeyValueString(key, QString("null")));
        }
        else if (value.isUndefined())
        {
            stringList.append(KeyValueString(key, QString("undefined")));
        }
        else
        {
            stringList.append(KeyValueString(key, value.toString()));
        }
    }

    const QString KeyValueString(const QString& key, const QString& value)
    {
        return key % ": " % value;
    }
}
