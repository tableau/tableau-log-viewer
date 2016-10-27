#include "statusbar.h"

StatusBar::StatusBar(QMainWindow* parent) :
    m_qbar(parent->statusBar()),
    m_statusLabel(new QLabel(parent))
{
    m_statusLabel->setContentsMargins(0, 0, 8, 0);
    m_qbar->addPermanentWidget(m_statusLabel);
}

void StatusBar::ShowMessage(const QString& message, int timeout)
{
    m_qbar->showMessage(message, timeout);
}

void StatusBar::SetRightLabelText(const QString& text)
{
    m_statusLabel->setText(text);
}
