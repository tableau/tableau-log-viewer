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

    void GetJson(const QJsonValue& value, QString& string, unsigned int level = 0);
    void GetJsonSingleLine(const QJsonValue& value, QString& string);
    bool GetJsonLiteral(const QJsonValue& value, QString& string);
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
        QString builder;
        switch (format)
        {
        case Notation::JSON:
            if (isSingleLine)
                GetJsonSingleLine(jsonValue, builder);
            else
                GetJson(jsonValue, builder);
            break;
        default:
            return "NOT IMPLEMENTED";
        }
        return builder;
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

    void GetJson(const QJsonValue& value, QString& string, unsigned int level)
    {
        if (GetJsonLiteral(value, string))
            return;

        QString indentation = "  ";
        indentation = indentation.repeated(level);

        if (value.isObject())
        {
            string.append("{\n");
            QJsonObject obj = value.toObject();
            int pendingKeys = obj.length();
            for (QJsonObject::ConstIterator iter = obj.constBegin(); iter != obj.constEnd(); ++iter)
            {
                string.append(indentation);
                string.append("  \"");
                string.append(iter.key());
                string.append("\": ");
                GetJson(iter.value(), string, level + 1);
                if (pendingKeys > 1)
                    string.append(",");
                string.append("\n");
                pendingKeys--;
            }
            string.append(indentation);
            string.append("}");
        }
        else if (value.isArray())
        {
            string.append("[\n");
            int pendingElements = value.toArray().count();
            for (const QJsonValue& itemValue : value.toArray())
            {
                string.append(indentation);
                string.append("  ");
                GetJson(itemValue, string, level + 1);
                if (pendingElements > 1)
                    string.append(", ");
                string.append("\n");
                pendingElements--;
            }
            string.append(indentation);
            string.append("]");
        }
    }

    void GetJsonSingleLine(const QJsonValue& value, QString& string)
    {
        if (GetJsonLiteral(value, string))
            return;

        if (value.isObject())
        {
            string.append("{ ");
            QJsonObject obj = value.toObject();
            int pendingKeys = obj.length();
            for (QJsonObject::ConstIterator iter = obj.constBegin(); iter != obj.constEnd(); ++iter)
            {
                string.append("\"");
                string.append(iter.key());
                string.append("\": ");
                GetJsonSingleLine(iter.value(), string);
                if (pendingKeys > 1)
                    string.append(", ");
                pendingKeys--;
            }
            string.append(" }");
        }
        else if (value.isArray())
        {
            string.append("[ ");
            int pendingElements = value.toArray().count();
            for (const QJsonValue& itemValue : value.toArray())
            {
                GetJsonSingleLine(itemValue, string);
                if (pendingElements > 1)
                    string.append(", ");
                pendingElements--;
            }
            string.append(" ]");
        }
    }

    // Literals are formatted the same in Single-line and multi-line
    bool GetJsonLiteral(const QJsonValue& value, QString& string)
    {
        if (value.isDouble())
        {
            // There are no ints in JSON, only doubles
            // Check if the double value is just an integer and format accordingly
            double intpart;
            const int decimals = (modf(value.toDouble(), &intpart) == 0) ? 0 : 3;

            string.append(QString::number(value.toDouble(), 'f', decimals));
        }
        else if (value.isBool())
        {
            string.append(QString(value.toBool() ? "true" : "false"));
        }
        else if (value.isNull())
        {
            string.append(QString("null"));
        }
        else if (value.isUndefined())
        {
            string.append(QString("undefined"));
        }
        else if (value.isString())
        {
            string.append("\"");
            string.append(value.toString().replace('\n', "\\n").replace('"', "\\\""));
            string.append("\"");
        }
        else
        {
            return false;
        }
        return true;
    }
}
