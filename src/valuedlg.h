#ifndef VALUEDLG_H
#define VALUEDLG_H

#include "treemodel.h"

#include <QDialog>
#include <QNetworkReply>
#include <QWebEngineView>

namespace Ui {
class ValueDlg;
}

class ValueDlg : public QDialog
{
    Q_OBJECT

public:
    explicit ValueDlg(QWidget *parent = 0);
    ~ValueDlg();
    void SetText(QString value, bool sqlHighlight);
    void SetQuery(QString queryXML);

    QString m_id;
    QString m_key;

private slots:
    void on_visualizeButton_clicked();
    void on_wrapTextCheck_clicked();
    void on_prevButton_clicked();
    void on_nextButton_clicked();
    void on_uploadFinished(QNetworkReply*);
    void on_zoomIn();
    void on_zoomOut();

signals:
    void next();
    void prev();

private:
    QString Process(QString in);
    QUrl GetUploadURL();
    QUrl GetVisualizeURL(QNetworkReply * const reply);
    void UploadQuery();
    void VisualizeQuery();
    void on_loadFinished(bool);

    Ui::ValueDlg *ui;
    QString m_queryXML;
    bool m_visualizationServiceEnable;
    QString m_visualizationServiceURL;
    QWebEngineView *m_view;
    bool m_reload;
};

#endif // VALUEDLG_H
