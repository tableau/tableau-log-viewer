#ifndef ZOOMABLETREEVIEW_H
#define ZOOMABLETREEVIEW_H

#include <QTreeView>
#include <QWidget>

class ZoomableTreeView: public QTreeView
{
    Q_OBJECT

public:
    ZoomableTreeView(QWidget *parent);
    void SetAutoResizeColumns(const std::vector<int>& columns);

protected:
    void wheelEvent(QWheelEvent *) override;
    void keyPressEvent(QKeyEvent *) override;

private:
    void ResizeFont(int delta);
    void ResizeColumns();

    int m_defaultFontSize = -1;
    std::vector<int> m_autoResizedColumns;

    const int MIN_FONT_SIZE = 7;
    const int MAX_FONT_SIZE = 24;
};

#endif // ZOOMABLETREEVIEW_H
