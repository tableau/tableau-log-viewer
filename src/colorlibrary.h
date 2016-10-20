#ifndef COLORLIBRARY_H
#define COLORLIBRARY_H

#include <QVector>
#include <QColor>
#include <QQueue>

// The purpose of this class is to provide some default colors for highlight filters.
// Default colors are stored in a queue and once a color is used, it is enqueued again.
// If a color is being used from a loaded filter, it will be moved to the end of the queue.
class ColorLibrary
{
public:
    ColorLibrary();
    ColorLibrary(QVector<QColor> usedColors);

    void Exclude(QVector<QColor> usedColors);
    QColor GetNextColor();

private:
    QQueue<QColor> m_colorLibrary;
    void AddColors(QVector<QColor> colors);
};
#endif // COLORLIBRARY_H
