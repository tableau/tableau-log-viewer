#include "statusbar.h"

StatusBar::StatusBar(QStatusBar* qbar, QLabel *statusLabel) :
    m_qbar(qbar),
    m_statusLabel(statusLabel)
{
}

void StatusBar::ShowMessage(const QString& message, int timeout)
{
    m_qbar->showMessage(message, timeout);
}

void StatusBar::SetRightLabelText(const QString& text)
{
    m_statusLabel->setText(text);
}
