#pragma once

#include <QTreeView>
class QSettings;

class ZoomableTreeView: public QTreeView
{
    Q_OBJECT

public:
    ZoomableTreeView(QWidget *parent);
    void SetAutoResizeColumns(const std::vector<int>& columns);
    void ResizeColumns();

    static void WriteSettings(QSettings& settings);
    static void ReadSettings(QSettings& settings);

protected:
    void wheelEvent(QWheelEvent *) override;
    void keyPressEvent(QKeyEvent *) override;

private:
    void ResizeFont(int delta);

    std::vector<int> m_autoResizedColumns;

    static qreal sm_savedFontPointSize;

    const int MIN_FONT_SIZE = 7;
    const int MAX_FONT_SIZE = 24;
};
