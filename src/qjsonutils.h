#pragma once

class QString;
class QJsonValue;

namespace QJsonUtils
{
    enum class Notation : unsigned int
    {
        Flat
    };

    enum class LineFormat : unsigned int
    {
        SingleLine,
        Free
    };

    QString Format(const QJsonValue& value, Notation format, LineFormat lineFormat = LineFormat::Free);
}
