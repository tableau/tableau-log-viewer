#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <QtWidgets>

class StatusBar
{
public:
    StatusBar(QMainWindow* parent);
    void ShowMessage(const QString& message, int timeout);
    void SetRightLabelText(const QString& text);

private:
    QStatusBar *m_qbar;
    QLabel *m_statusLabel;
};

#endif // STATUSBAR_H
