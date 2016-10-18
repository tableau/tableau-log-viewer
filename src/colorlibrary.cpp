#include "colorlibrary.h"

const static QVector<QColor> DefaultColors =
{
    QColor("#f2ec99"),
    QColor("#bfe4f3"),
    QColor("#bfe968"),
    QColor("#c6caf8"),
    QColor("#eec757"),
    QColor("#9994f1"),
    QColor("#edb0e8"),
    QColor("#cebff3"),
    QColor("#a5f0c6"),
    QColor("#f5b0b4")
};

ColorLibrary::ColorLibrary()
{
    m_colorLibrary = new QQueue<QColor>();
    AddColors(DefaultColors);
}

ColorLibrary::ColorLibrary(QVector<QColor> usedColors)
{
    m_colorLibrary = new QQueue<QColor>();
    QVector<QColor> colorsToAdd;
    for(const auto& color : DefaultColors)
    {
        if (!usedColors.contains(color))
            colorsToAdd.push_back(color);
    }
    AddColors(colorsToAdd);
}

void ColorLibrary::AddColors(QVector<QColor> colors)
{
    for (const auto& color : colors)
    {
        m_colorLibrary->enqueue(color);
    }
}

QColor ColorLibrary::GetNextColor()
{
    QColor result = m_colorLibrary->head();
    auto color = m_colorLibrary->dequeue();
    m_colorLibrary->enqueue(color);
    return result;
}


