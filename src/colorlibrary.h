#ifndef COLORLIBRARY_H
#define COLORLIBRARY_H

#include <QVector>
#include <QColor>
#include <QQueue>

class ColorLibrary
{
public:
    ColorLibrary();
    ColorLibrary(QVector<QColor> usedColors);
    QColor GetNextColor();

private:
    QQueue<QColor> * m_colorLibrary;
    void AddColors(QVector<QColor> colors);
};
#endif // COLORLIBRARY_H
