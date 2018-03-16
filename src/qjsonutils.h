#pragma once

class QString;
class QStringList;
class QJsonValue;

namespace QJsonUtils
{
    enum class Notation : unsigned int
    {
        Flat,
        JSON,
        YAML
    };

    enum class LineFormat : unsigned int
    {
        SingleLine,
        Free
    };

    QString Format(const QJsonValue& value, Notation format, LineFormat lineFormat = LineFormat::Free);

    QStringList GetNotationNames();

    Notation GetNotationFromName(const QString& notationName);
    QString GetNameForNotation(Notation notation);
}
