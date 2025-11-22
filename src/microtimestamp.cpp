#include "microtimestamp.h"
#include <QRegularExpression>
#include <QStringList>

// Global static regex for microsecond timestamp parsing
Q_GLOBAL_STATIC(QRegularExpression, microsecondRegex, (R"(^(\d{4})-(\d{2})-(\d{2})T(\d{2}):(\d{2}):(\d{2})\.(\d{6})$)"));

MicroTimestamp::MicroTimestamp()
    : m_microseconds(0), m_valid(false), m_hasMicros(false)
{
}

MicroTimestamp::MicroTimestamp(const QString &timestampString)
    : m_microseconds(0), m_valid(false), m_hasMicros(false)
{
    parseTimestamp(timestampString);
}

MicroTimestamp::MicroTimestamp(const QDateTime &dateTime)
    : m_dateTime(dateTime), m_microseconds(0), m_valid(dateTime.isValid()), m_hasMicros(false)
{
}

void MicroTimestamp::parseTimestamp(const QString &timestampString)
{
    if (timestampString.isEmpty()) {
        m_valid = false;
        return;
    }

    // Check for microsecond format: yyyy-MM-ddTHH:mm:ss.zzzzzz (26 chars)
    if (timestampString.length() == 26) {
        // Parse the microsecond timestamp manually
        QRegularExpressionMatch match = microsecondRegex()->match(timestampString);

        if (match.hasMatch()) {
            int year = match.captured(1).toInt();
            int month = match.captured(2).toInt();
            int day = match.captured(3).toInt();
            int hour = match.captured(4).toInt();
            int minute = match.captured(5).toInt();
            int second = match.captured(6).toInt();
            QString microStr = match.captured(7);

            // Create QDateTime with millisecond precision
            int milliseconds = microStr.left(3).toInt();
            m_microseconds = microStr.right(3).toInt(); // Additional microseconds

            QDate date(year, month, day);
            QTime time(hour, minute, second, milliseconds);
            m_dateTime = QDateTime(date, time);

            m_valid = m_dateTime.isValid();
            m_hasMicros = true;
            return;
        }
    }

    // Try standard millisecond format: yyyy-MM-ddTHH:mm:ss.zzz (23 chars)
    if (timestampString.length() == 23) {
        m_dateTime = QDateTime::fromString(timestampString, "yyyy-MM-ddTHH:mm:ss.zzz");
        m_microseconds = 0;
        m_valid = m_dateTime.isValid();
        m_hasMicros = false;
        return;
    }

    // Try other common formats
    QStringList formats = {
        "yyyy-MM-ddTHH:mm:ss",
        "yyyy-MM-dd HH:mm:ss.zzz",
        "yyyy-MM-dd HH:mm:ss"
    };

    for (const QString &format : formats) {
        m_dateTime = QDateTime::fromString(timestampString, format);
        if (m_dateTime.isValid()) {
            m_microseconds = 0;
            m_valid = true;
            m_hasMicros = false;
            return;
        }
    }

    m_valid = false;
}

bool MicroTimestamp::operator<(const MicroTimestamp &other) const
{
    if (!m_valid || !other.m_valid) {
        return !m_valid && other.m_valid; // Invalid timestamps sort first
    }

    if (m_dateTime != other.m_dateTime) {
        return m_dateTime < other.m_dateTime;
    }

    // If QDateTime is equal, compare microseconds
    return m_microseconds < other.m_microseconds;
}

bool MicroTimestamp::operator<=(const MicroTimestamp &other) const
{
    return *this < other || *this == other;
}

bool MicroTimestamp::operator>(const MicroTimestamp &other) const
{
    return !(*this <= other);
}

bool MicroTimestamp::operator>=(const MicroTimestamp &other) const
{
    return !(*this < other);
}

bool MicroTimestamp::operator==(const MicroTimestamp &other) const
{
    if (!m_valid || !other.m_valid) {
        return m_valid == other.m_valid;
    }

    return m_dateTime == other.m_dateTime && m_microseconds == other.m_microseconds;
}

bool MicroTimestamp::operator!=(const MicroTimestamp &other) const
{
    return !(*this == other);
}

QString MicroTimestamp::toString(const QString &format) const
{
    if (!m_valid) {
        return QString();
    }

    QString result = m_dateTime.toString(format);

    // If format contains microsecond placeholder and we have microseconds
    if (m_hasMicros && format.contains("zzzzzz")) {
        // Replace the millisecond part with full microsecond precision
        QString msStr = QString("%1").arg(m_dateTime.time().msec(), 3, 10, QChar('0'));
        QString microStr = QString("%1%2").arg(msStr).arg(m_microseconds, 3, 10, QChar('0'));
        result.replace(msStr, microStr);
    }

    return result;
}

QString MicroTimestamp::toDisplayString(bool includeDate) const
{
    if (!m_valid) {
        return QString();
    }

    if (m_hasMicros) {
        // Custom formatting for microsecond display with half space separator
        QString dateStr = m_dateTime.date().toString("MM/dd/yyyy");
        QString timeStr = m_dateTime.time().toString("hh:mm:ss");
        QString millisStr = QString("%1").arg(m_dateTime.time().msec(), 3, 10, QChar('0'));
        QString microsStr = QString("%1").arg(m_microseconds, 3, 10, QChar('0'));

        // Use regular space to separate milliseconds from microseconds
        QString microStr = QString("%1 %2").arg(millisStr, microsStr);

        if (includeDate) {
            return QString("%1 - %2.%3").arg(dateStr, timeStr, microStr);
        } else {
            return QString("%1.%2").arg(timeStr, microStr);
        }
    } else {
        // Standard formatting for millisecond display
        if (includeDate) {
            return m_dateTime.toString("MM/dd/yyyy - hh:mm:ss.zzz");
        } else {
            return m_dateTime.toString("hh:mm:ss.zzz");
        }
    }
}

QString MicroTimestamp::toCopyString(bool includeDate) const
{
    if (!m_valid) {
        return QString();
    }

    if (m_hasMicros) {
        // Format for copying without space separator
        QString dateStr = m_dateTime.date().toString("MM/dd/yyyy");
        QString timeStr = m_dateTime.time().toString("hh:mm:ss");
        QString microStr = QString("%1%2")
            .arg(m_dateTime.time().msec(), 3, 10, QChar('0'))
            .arg(m_microseconds, 3, 10, QChar('0'));

        if (includeDate) {
            return QString("%1 - %2.%3").arg(dateStr, timeStr, microStr);
        } else {
            return QString("%1.%2").arg(timeStr, microStr);
        }
    } else {
        // Standard formatting for millisecond display
        if (includeDate) {
            return m_dateTime.toString("MM/dd/yyyy - hh:mm:ss.zzz");
        } else {
            return m_dateTime.toString("hh:mm:ss.zzz");
        }
    }
}

QDateTime MicroTimestamp::toDateTime() const
{
    return m_dateTime;
}

qint64 MicroTimestamp::toMSecsSinceEpoch() const
{
    if (!m_valid) {
        return 0;
    }
    return m_dateTime.toMSecsSinceEpoch();
}

qint64 MicroTimestamp::toMicroSecsSinceEpoch() const
{
    if (!m_valid) {
        return 0;
    }
    return m_dateTime.toMSecsSinceEpoch() * 1000 + m_microseconds;
}

bool MicroTimestamp::isValid() const
{
    return m_valid;
}

bool MicroTimestamp::hasMicroseconds() const
{
    return m_hasMicros;
}

void MicroTimestamp::registerMetaType()
{
    qRegisterMetaType<MicroTimestamp>("MicroTimestamp");
}


static bool microTimestampRegistered = []() {
    MicroTimestamp::registerMetaType();
    return true;
}();
