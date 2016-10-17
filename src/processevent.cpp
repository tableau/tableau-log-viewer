#include "processevent.h"

#include "options.h"
#include "pathhelper.h"

#include <QBitArray>
#include <QJsonDocument>
#include <QSettings>

namespace ProcessEvent
{
    QJsonObject ProcessLogEventMessage(int index, QString message, const QString& fileName)
    {
        Options& options = Options::GetInstance();
        QStringList m_SkippedText = options.getSkippedText();
        QBitArray m_SkippedState = options.getSkippedState();

        QJsonDocument jsonDoc;
        if (!message.startsWith("{"))
        {
            auto str = QString("{\"idx\": %1, \"k\": \"\", \"v\": \"%2\"}")
                    .arg(QString::number(index), message.replace("\\", "\\\\"));
            jsonDoc = QJsonDocument::fromJson(str.toUtf8());
        }
        else
        {
            message.insert(1, QString("\"file\": \"%1\",").arg(fileName));
            message.insert(1, QString("\"idx\": %1,").arg(index));
            jsonDoc = QJsonDocument::fromJson(message.toUtf8());
            if (jsonDoc.object().contains("k")
                    && m_SkippedText.contains(jsonDoc.object()["k"].toString())
                    && m_SkippedState[m_SkippedText.indexOf(jsonDoc.object()["k"].toString(), 0)])
            {
                jsonDoc = QJsonDocument();
            }
        }
        return jsonDoc.object();
    }
}
