#ifndef MICROTIMESTAMP_H
#define MICROTIMESTAMP_H

#include <QDateTime>
#include <QString>
#include <QVariant>

class MicroTimestamp
{
public:
    MicroTimestamp();
    explicit MicroTimestamp(const QString &timestampString);
    explicit MicroTimestamp(const QDateTime &dateTime);
    
    // Comparison operators for sorting
    bool operator<(const MicroTimestamp &other) const;
    bool operator<=(const MicroTimestamp &other) const;
    bool operator>(const MicroTimestamp &other) const;
    bool operator>=(const MicroTimestamp &other) const;
    bool operator==(const MicroTimestamp &other) const;
    bool operator!=(const MicroTimestamp &other) const;
    
    // Display formatting
    QString toString(const QString &format) const;
    QString toDisplayString(bool includeDate = true) const;
    QString toCopyString(bool includeDate = true) const;  // For copying without space separator
    
    // Conversion functions
    QDateTime toDateTime() const;
    qint64 toMSecsSinceEpoch() const;
    qint64 toMicroSecsSinceEpoch() const;
    
    // Validation
    bool isValid() const;
    bool hasMicroseconds() const;
    
    // For QVariant storage
    static void registerMetaType();

private:
    QDateTime m_dateTime;
    int m_microseconds;  // Additional microseconds (0-999)
    bool m_valid;
    bool m_hasMicros;
    
    void parseTimestamp(const QString &timestampString);
};

Q_DECLARE_METATYPE(MicroTimestamp)

#endif // MICROTIMESTAMP_H
