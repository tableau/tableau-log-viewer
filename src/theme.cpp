#include "theme.h"
#include "themeutils.h"

#include <memory>
#include <QApplication>
#include <QPalette>
#include <QStyle>
#include <QStyleFactory>
#include <QRegularExpression>
#include <QWidget>

namespace
{

std::unique_ptr<QPalette> GetNativePalette(QWidget* widget)
{
    return std::make_unique<QPalette>(widget->style()->standardPalette());
}

std::unique_ptr<QPalette> GetDarkFusionPalette(QWidget*)
{
    // Theme by GitHub user QuantumCD, Dark Fusion Palette
    // https://gist.github.com/QuantumCD/6245215
    auto palette = std::make_unique<QPalette>();
    palette->setColor(QPalette::Window,          QColor(0x353535));
    palette->setColor(QPalette::WindowText,      QColor(0xf1f1f1));
    palette->setColor(QPalette::Base,            QColor(0x202020));
    palette->setColor(QPalette::AlternateBase,   QColor(0x353535));
    palette->setColor(QPalette::ToolTipBase,     QColor(0x353535));
    palette->setColor(QPalette::ToolTipText,     QColor(0xf1f1f1));
    palette->setColor(QPalette::Text,            QColor(0xf1f1f1));
    palette->setColor(QPalette::Button,          QColor(0x353535));
    palette->setColor(QPalette::ButtonText,      QColor(0xf1f1f1));
    palette->setColor(QPalette::BrightText,      Qt::red);
    palette->setColor(QPalette::Link,            QColor(0x2a82da));

    palette->setColor(QPalette::Highlight,       QColor(0x2a82da));
    palette->setColor(QPalette::HighlightedText, Qt::black);
    palette->setColor(QPalette::Light,           QColor(0x303030));

    palette->setColor(QPalette::Disabled, QPalette::Text,       QColor(0x909090));
    palette->setColor(QPalette::Disabled, QPalette::Button,     QColor(0x202020));
    palette->setColor(QPalette::Disabled, QPalette::ButtonText, QColor(0x909090));
    palette->setColor(QPalette::Disabled, QPalette::Base,       QColor(0x353535));
    return palette;
}

std::unique_ptr<QPalette> GetSolarizedLightPalette(QWidget*)
{
    // Colors from the Solarized Palette, light mode, by Ethan Schoonover
    // https://github.com/altercation/solarized
    auto palette = std::make_unique<QPalette>();
    palette->setColor(QPalette::Window,          QColor(0xe6ddc1));
    palette->setColor(QPalette::WindowText,      QColor(0x657b83));
    palette->setColor(QPalette::Base,            QColor(0xfdf6e3));
    palette->setColor(QPalette::AlternateBase,   QColor(0xeee8d5));
    palette->setColor(QPalette::ToolTipBase,     QColor(0xeee8d5));
    palette->setColor(QPalette::ToolTipText,     QColor(0x657b83));
    palette->setColor(QPalette::Text,            QColor(0x657b83));
    palette->setColor(QPalette::Button,          QColor(0xe6ddc1));
    palette->setColor(QPalette::ButtonText,      QColor(0x657b83));
    palette->setColor(QPalette::BrightText,      Qt::red);
    palette->setColor(QPalette::Link,            QColor(0x2aa198));

    palette->setColor(QPalette::Highlight,       QColor(0x268bd2));
    palette->setColor(QPalette::HighlightedText, Qt::black);
    palette->setColor(QPalette::Light,           QColor(0xeee8d5));

    palette->setColor(QPalette::Disabled, QPalette::Text,       QColor(0x93a1a1));
    palette->setColor(QPalette::Disabled, QPalette::Button,     QColor(0xfdf6e3));
    palette->setColor(QPalette::Disabled, QPalette::ButtonText, QColor(0x93a1a1));
    palette->setColor(QPalette::Disabled, QPalette::Base,       QColor(0xe6ddc1));
    return palette;
}

std::unique_ptr<QPalette> GetSolarizedDarkPalette(QWidget*)
{
    // Colors from the Solarized Palette, dark mode, by Ethan Schoonover
    // https://github.com/altercation/solarized
    auto palette = std::make_unique<QPalette>();
    palette->setColor(QPalette::Window,          QColor(0x001e26));
    palette->setColor(QPalette::WindowText,      QColor(0x839496));
    palette->setColor(QPalette::Base,            QColor(0x002b36));
    palette->setColor(QPalette::AlternateBase,   QColor(0x073642));
    palette->setColor(QPalette::ToolTipBase,     QColor(0x073642));
    palette->setColor(QPalette::ToolTipText,     QColor(0x839496));
    palette->setColor(QPalette::Text,            QColor(0x839496));
    palette->setColor(QPalette::Button,          QColor(0x001e26));
    palette->setColor(QPalette::ButtonText,      QColor(0x839496));
    palette->setColor(QPalette::BrightText,      Qt::red);
    palette->setColor(QPalette::Link,            QColor(0x2aa198));

    palette->setColor(QPalette::Highlight,       QColor(0x268bd2));
    palette->setColor(QPalette::HighlightedText, Qt::black);
    palette->setColor(QPalette::Light,           QColor(0x073642));

    palette->setColor(QPalette::Disabled, QPalette::Text,       QColor(0x586e75));
    palette->setColor(QPalette::Disabled, QPalette::Button,     QColor(0x002b36));
    palette->setColor(QPalette::Disabled, QPalette::ButtonText, QColor(0x586e75));
    palette->setColor(QPalette::Disabled, QPalette::Base,       QColor(0x001e26));
    return palette;
}

std::unique_ptr<QPalette> GetEvergreenStatePalette(QWidget*)
{
    auto palette = std::make_unique<QPalette>();
    palette->setColor(QPalette::Window,          QColor(0x4d6625));
    palette->setColor(QPalette::WindowText,      QColor(0xf1f1f1));
    palette->setColor(QPalette::Base,            QColor(0x2e3c18));
    palette->setColor(QPalette::AlternateBase,   QColor(0xffff7f));
    palette->setColor(QPalette::ToolTipBase,     QColor(0x4b3a29));
    palette->setColor(QPalette::ToolTipText,     QColor(0xf1f1f1));
    palette->setColor(QPalette::Text,            QColor(0xdadada));
    palette->setColor(QPalette::Button,          QColor(0x4d6625));
    palette->setColor(QPalette::ButtonText,      QColor(0xf1f1f1));
    palette->setColor(QPalette::BrightText,      Qt::red);
    palette->setColor(QPalette::Link,            QColor(0x2a82da));

    palette->setColor(QPalette::Highlight,       QColor(0xccd970));
    palette->setColor(QPalette::HighlightedText, Qt::black);
    palette->setColor(QPalette::Light,           QColor(0x303030));

    palette->setColor(QPalette::Disabled, QPalette::Text,       QColor(0x909090));
    palette->setColor(QPalette::Disabled, QPalette::Button,     QColor(0x2e3c18));
    palette->setColor(QPalette::Disabled, QPalette::ButtonText, QColor(0x909090));
    palette->setColor(QPalette::Disabled, QPalette::Base,       QColor(0x4d6625));
    return palette;
}

using ThemeFunctionPtr = std::unique_ptr<QPalette> (*)(QWidget* widget);

struct ThemeEntry
{
    bool isFusionStyle;
    bool isDark;
    ThemeFunctionPtr function;
};

static QMap<QString, ThemeEntry> themeMap = {
    {"Native", {false, false, GetNativePalette}},
    {"Dark Fusion", {true, true, GetDarkFusionPalette}},
    {"Solarized Light", {true, false, GetSolarizedLightPalette}},
    {"Solarized Dark", {true, true, GetSolarizedDarkPalette}},
    {"Evergreen State", {true, true, GetEvergreenStatePalette}},
};

} // namespace

Theme::Theme(bool isFusionStyle, bool isDark, std::unique_ptr<QPalette> palette):
    m_isFusionStyle(isFusionStyle),
    m_isDark(isDark),
    m_palette(std::move(palette))
{
}

QString Theme::sm_defaultStyle;
QString Theme::GetDefaultStyle()
{
    if (sm_defaultStyle.isEmpty())
    {
        // Extract the style name from the active style class name
        // Example: QWindowsVistaStyle -> WindowsVista
        QRegularExpression regExp("^.(.*)\\+?Style$");
        sm_defaultStyle = QApplication::style()->metaObject()->className();
        auto match = regExp.match(sm_defaultStyle);

        if (match.hasMatch())
        {
            sm_defaultStyle = match.captured(1);
        }
        else
        {
            // If this fails (maybe in a OS that we didn't test), fallback to the Fusion style
            sm_defaultStyle = "Fusion";
        }
    }
    return sm_defaultStyle;
}

QStringList Theme::GetThemeNames()
{
    return themeMap.keys();
}

std::unique_ptr<Theme> Theme::ThemeFactory(const QString& themeName, QWidget* widget)
{
    if (themeMap.contains(themeName))
    {
        const ThemeEntry& entry = themeMap[themeName];
        return std::make_unique<Theme>(entry.isFusionStyle, entry.isDark, entry.function(widget));
    }

    return nullptr;
}

void Theme::Activate()
{
    // For a strange reason (maybe a bug in Qt?), we have to do this twice
    // to propagate the style and palette to all the controls. Doing it only once
    // leaves some controls in the old style.
    for (int i = 0; i < 2; i++)
    {
        QString defaultStyle = GetDefaultStyle();
        QStyle* style = this->m_isFusionStyle ?
            QStyleFactory::create("Fusion") :
            QStyleFactory::create(defaultStyle);
        qApp->setStyle(style);
        qApp->setPalette(*m_palette);
        if (m_isDark)
        {
            ThemeUtils::SetDarkIconSet();
        }
        else
        {
            ThemeUtils::SetLightIconSet();
        }
    }
}
