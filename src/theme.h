#pragma once

#include <memory>

class QColor;
class QPalette;
class QString;
class QStyle;
class QWidget;

class Theme {
private:
    static std::unique_ptr<QPalette> GetNativePalette(QWidget* widget);
    static std::unique_ptr<QPalette> GetDarkPalette();
    static std::unique_ptr<QPalette> GetSolarizedLightPalette();
    static std::unique_ptr<QPalette> GetSolarizedDarkPalette();
    static QString GetDefaultStyle();
    static QString sm_defaultStyle;

    bool m_isFusionStyle;
    bool m_isDark;
    std::unique_ptr<QPalette> m_palette;
public:
    Theme(bool isFusionStyle, bool isDark, std::unique_ptr<QPalette> palette);
    static std::unique_ptr<Theme> ThemeFactory(const QString& themeName, QWidget* widget);
    void Activate();
};
