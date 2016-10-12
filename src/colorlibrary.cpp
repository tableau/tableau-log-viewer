#include "colorlibrary.h"

ColorLibrary::ColorLibrary()
{
    m_colorLibrary = new QQueue<QColor>();
    m_colorLibrary->enqueue(QColor("#f2ec99"));
    m_colorLibrary->enqueue(QColor("#bfe4f3"));
    m_colorLibrary->enqueue(QColor("#bfe968"));
    m_colorLibrary->enqueue(QColor("#c6caf8"));
    m_colorLibrary->enqueue(QColor("#eec757"));
    m_colorLibrary->enqueue(QColor("#9994f1"));
    m_colorLibrary->enqueue(QColor("#edb0e8"));
    m_colorLibrary->enqueue(QColor("#cebff3"));
    m_colorLibrary->enqueue(QColor("#a5f0c6"));
    m_colorLibrary->enqueue(QColor("#f5b0b4"));
}

QColor ColorLibrary::GetNextColor()
{
    QColor result = m_colorLibrary->head();
    auto color = m_colorLibrary->dequeue();
    m_colorLibrary->enqueue(color);
    return result;
}
