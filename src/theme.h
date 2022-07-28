#pragma once

#include <memory>
#include <qcontainerfwd.h>

class QPalette;
class QString;
class QWidget;

class Theme {
private:
    static QString GetDefaultStyle();
    static QString sm_defaultStyle;

    bool m_isFusionStyle;
    bool m_isDark;
    std::unique_ptr<QPalette> m_palette;
public:
    Theme(bool isFusionStyle, bool isDark, std::unique_ptr<QPalette> palette);
    static QStringList GetThemeNames();
    static std::unique_ptr<Theme> ThemeFactory(const QString& themeName, QWidget* widget);
    void Activate();
};
