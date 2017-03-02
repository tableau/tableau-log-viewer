#include "theme.h"
#include "themeutils.h"

#include <memory>
#include <QApplication>
#include <QPalette>
#include <QStyle>
#include <QStyleFactory>
#include <QWidget>

std::unique_ptr<QPalette> Theme::GetNativePalette(QWidget* widget)
{
    return std::unique_ptr<QPalette>(
        new QPalette(widget->style()->standardPalette())
    );
}

std::unique_ptr<QPalette> Theme::GetDarkPalette()
{
    // Theme by GitHub user QuantumCD, Dark Fusion Palette
    // https://gist.github.com/QuantumCD/6245215
    auto palette = std::make_unique<QPalette>();
    palette->setColor(QPalette::Window,          QColor(0x353535));
    palette->setColor(QPalette::WindowText,      QColor(0xf1f1f1));
    palette->setColor(QPalette::Base,            QColor(0x202020));
    palette->setColor(QPalette::AlternateBase,   QColor(0x353535));
    palette->setColor(QPalette::ToolTipBase,     QColor(0xf1f1f1));
    palette->setColor(QPalette::ToolTipText,     QColor(0xf1f1f1));
    palette->setColor(QPalette::Text,            QColor(0xf1f1f1));
    palette->setColor(QPalette::Button,          QColor(0x353535));
    palette->setColor(QPalette::ButtonText,      QColor(0xf1f1f1));
    palette->setColor(QPalette::BrightText,      Qt::red);
    palette->setColor(QPalette::Link,            QColor(0x2a82da));

    palette->setColor(QPalette::Highlight,       QColor(0x2a82da));
    palette->setColor(QPalette::HighlightedText, Qt::black);
    return palette;
}

std::unique_ptr<QPalette> Theme::GetSolarizedLightPalette()
{
    // Colors from the Solarized Palette, light mode, by Ethan Schoonover
    // https://github.com/altercation/solarized
    auto palette = std::make_unique<QPalette>();
    palette->setColor(QPalette::Window,          QColor(0xe6ddc1));
    palette->setColor(QPalette::WindowText,      QColor(0x657b83));
    palette->setColor(QPalette::Base,            QColor(0xfdf6e3));
    palette->setColor(QPalette::AlternateBase,   QColor(0xeee8d5));
    palette->setColor(QPalette::ToolTipBase,     QColor(0x657b83));
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
    palette->setColor(QPalette::Disabled, QPalette::Button,     QColor(0x93a1a1));
    palette->setColor(QPalette::Disabled, QPalette::ButtonText, QColor(0xeee8d5));
    palette->setColor(QPalette::Disabled, QPalette::Base,       QColor(0xe6ddc1));
    return palette;
}

std::unique_ptr<QPalette> Theme::GetSolarizedDarkPalette()
{
    // Colors from the Solarized Palette, dark mode, by Ethan Schoonover
    // https://github.com/altercation/solarized
    auto palette = std::make_unique<QPalette>();
    palette->setColor(QPalette::Window,          QColor(0x001e26));
    palette->setColor(QPalette::WindowText,      QColor(0x839496));
    palette->setColor(QPalette::Base,            QColor(0x002b36));
    palette->setColor(QPalette::AlternateBase,   QColor(0x073642));
    palette->setColor(QPalette::ToolTipBase,     QColor(0x839496));
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
    palette->setColor(QPalette::Disabled, QPalette::Button,     QColor(0x586e75));
    palette->setColor(QPalette::Disabled, QPalette::ButtonText, QColor(0x073642));
    palette->setColor(QPalette::Disabled, QPalette::Base,       QColor(0x001e26));
    return palette;
}

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
        QRegExp regExp(".(.*)\\+?Style");
        sm_defaultStyle = QApplication::style()->metaObject()->className();

        if (regExp.exactMatch(sm_defaultStyle))
        {
            sm_defaultStyle = regExp.cap(1);
        }
        else
        {
            // If this fails (maybe in a OS that we didn't test), fallback to the Fusion style
            sm_defaultStyle = "Fusion";
        }
    }
    return sm_defaultStyle;
}

std::unique_ptr<Theme> Theme::ThemeFactory(const QString& themeName, QWidget* widget)
{
    if (themeName == "Native")
    {
        return std::make_unique<Theme>(false, false, GetNativePalette(widget));
    }
    else if (themeName == "Dark Fusion")
    {
        return std::make_unique<Theme>(true, true, GetDarkPalette());
    }
    else if (themeName == "Solarized Light")
    {
        return std::make_unique<Theme>(true, false, GetSolarizedLightPalette());
    }
    else if (themeName == "Solarized Dark")
    {
        return std::make_unique<Theme>(true, true, GetSolarizedDarkPalette());
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
