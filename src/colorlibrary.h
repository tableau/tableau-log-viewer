#ifndef COLORLIBRARY_H
#define COLORLIBRARY_H

#include <QColor>
#include <QQueue>

class ColorLibrary
{
public:
    ColorLibrary();
    QColor GetNextColor();
    void AddColor(QColor color);

private:
    QQueue<QColor> * m_colorLibrary;
};
#endif // COLORLIBRARY_H
