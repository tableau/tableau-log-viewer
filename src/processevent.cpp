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

        if (!message.startsWith("{"))
        {
            QJsonObject obj;
            obj["idx"] = index;
            obj["file"] = fileName;
            obj["k"] = "";
            obj["v"] = message;
            return obj;
        }
        else
        {
            message.insert(1, QString("\"file\": \"%1\",").arg(fileName));
            message.insert(1, QString("\"idx\": %1,").arg(index));
            QJsonDocument jsonDoc = QJsonDocument::fromJson(message.toUtf8());
            if (jsonDoc.object().contains("k")
                    && m_SkippedText.contains(jsonDoc.object()["k"].toString())
                    && m_SkippedState[m_SkippedText.indexOf(jsonDoc.object()["k"].toString(), 0)])
            {
                jsonDoc = QJsonDocument();
            }
            return jsonDoc.object();
        }
    }
}
