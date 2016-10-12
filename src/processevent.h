#ifndef PROCESSEVENT_H
#define PROCESSEVENT_H

#include <QJsonObject>
#include <QString>

namespace ProcessEvent
{
    QJsonObject ProcessLogEventMessage(int index, QString message, const QString& fileName);
}

#endif // PROCESSEVENT_H
