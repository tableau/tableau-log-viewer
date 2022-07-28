#pragma once

#include <QDialog>
#include <QJsonValue>
#include "qjsonutils.h"
class QNetworkReply;
class QSettings;
class QWebEngineView;

namespace Ui {
class ValueDlg;
}

class ValueDlg : public QDialog
{
    Q_OBJECT

public:
    explicit ValueDlg(QWidget *parent = 0);
    ~ValueDlg();
    void SetContent(QString id, QString key, QJsonValue value);

    static void WriteSettings(QSettings& settings);
    static void ReadSettings(QSettings& settings);

protected:
    void UpdateValueBox();
    void reject() override;

private slots:
    void on_visualizeButton_clicked();
    void on_wrapTextCheck_clicked();
    void on_prevButton_clicked();
    void on_notationComboBox_currentTextChanged(const QString& newValue);
    void on_nextButton_clicked();
    void on_uploadFinished(QNetworkReply*);
    void on_loadFinished(bool);
    void on_zoomIn();
    void on_zoomOut();

signals:
    void next();
    void prev();

private:
    QUrl GetUploadURL();
    QUrl GetVisualizeURL(QNetworkReply * const reply);
    void UploadQuery();
    void VisualizeQuery();
    void SetWrapping(const bool wrapText);

    Ui::ValueDlg *ui;
    QString m_id;
    QString m_key;
    QString m_queryPlan;
    QJsonValue m_value;
    bool m_visualizationServiceEnable;
    QString m_visualizationServiceURL;
    QWebEngineView *m_view;
    bool m_reload;

    static QByteArray sm_savedGeometry;
    static qreal sm_savedFontPointSize;
    static bool sm_savedWrapText;
    static QJsonUtils::Notation sm_notation;
};
