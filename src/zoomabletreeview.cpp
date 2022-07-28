#include "zoomabletreeview.h"
#include "options.h"

#include <QApplication>
#include <QDebug>
#include <QFontDatabase>
#include <QSettings>
#include <QWheelEvent>

qreal ZoomableTreeView::sm_savedFontPointSize { 0 };

ZoomableTreeView::ZoomableTreeView(QWidget *parent)
    :QTreeView(parent)
{
    QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    if (sm_savedFontPointSize > 0) {
       fixedFont.setPointSizeF(sm_savedFontPointSize);
    }
    this->setFont(fixedFont);
}

void ZoomableTreeView::SetAutoResizeColumns(const std::vector<int>& columns)
{
    m_autoResizedColumns = columns;
    ResizeColumns();
}

void ZoomableTreeView::ResizeColumns()
{
    // Resize column width. Not using QHeaderView::setSectionResizeMode because
    // it does not allow users to change size and only looks at 1000 rows by default.
    for (int col : m_autoResizedColumns)
    {
        resizeColumnToContents(col);
    }
}

void ZoomableTreeView::ResizeFont(int delta)
{
    QFont font = this->font();

    // Set the font size, make sure it fits in a reasonable range
    int pointSize;
    if (delta == 0)
    {
        QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        pointSize = font.pointSize();
    }
    else
    {
        pointSize = font.pointSize() + delta;
        pointSize = std::max(std::min(pointSize, MAX_FONT_SIZE), MIN_FONT_SIZE);
    }
    font.setPointSize(pointSize);
    this->setFont(font);
    sm_savedFontPointSize = pointSize;

    ResizeColumns();
}

void ZoomableTreeView::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier)
    {
        // Zooom-in or Zoom-out if CTRL key is used (Command key on Mac)
        int fontChange = (event->pixelDelta().y() > 0) ? 1 : -1;
        ResizeFont(fontChange);
        event->accept();
    }
    else
    {
        // Use default wheel event otherwise
        QTreeView::wheelEvent(event);
    }
}

void ZoomableTreeView::keyPressEvent(QKeyEvent* event)
{
    if ((event->key() == Qt::Key_Plus || event->key() == Qt::Key_Equal) &&
        (QApplication::keyboardModifiers() & Qt::ControlModifier))
    {
        ResizeFont(1);
    }
    else if ((event->key() == Qt::Key_Minus) && (QApplication::keyboardModifiers() & Qt::ControlModifier))
    {
        ResizeFont(-1);
    }
    else if ((event->key() == Qt::Key_0) && (QApplication::keyboardModifiers() & Qt::ControlModifier))
    {
        ResizeFont(0);
    }
    else
    {
        QTreeView::keyPressEvent(event);
    }
}

void ZoomableTreeView::WriteSettings(QSettings& settings)
{
    settings.beginGroup("TreeView");
    settings.setValue("fontPointSize", sm_savedFontPointSize);
    settings.endGroup();
}

void ZoomableTreeView::ReadSettings(QSettings& settings)
{
    settings.beginGroup("TreeView");
    sm_savedFontPointSize = settings.value("fontPointSize").toReal();
    settings.endGroup();
}
