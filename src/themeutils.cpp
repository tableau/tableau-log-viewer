#include "theme.h"
#include "themeutils.h"

#include <algorithm>
#include <cmath>
#include <QColor>
#include <QFileSelector>
#include <QPalette>

void ThemeUtils::SwitchTheme(const QString& themeName, QWidget* widget)
{
    auto theme = Theme::ThemeFactory(themeName, widget);
    theme->Activate();
}

double ThemeUtils::Luminance(QColor color)
{
    // https://www.w3.org/TR/WCAG/#relativeluminancedef
    double r = color.redF();
    double g = color.greenF();
    double b = color.blueF();
    r = (r <= 0.03928) ? r/12.93 : std::pow((r+0.055)/1.055, 2.4);
    g = (g <= 0.03928) ? g/12.93 : std::pow((g+0.055)/1.055, 2.4);
    b = (b <= 0.03928) ? b/12.93 : std::pow((b+0.055)/1.055, 2.4);
    return 0.2126 * r + 0.7152 * g + 0.0722 * b;
}

double ThemeUtils::ContrastRatio(QColor a, QColor b)
{
    // https://www.w3.org/TR/WCAG/#contrast-ratiodef
    double luminance1 = ThemeUtils::Luminance(a);
    double luminance2 = ThemeUtils::Luminance(b);
    double light = std::max(luminance1, luminance2);
    double dark = std::min(luminance1, luminance2);
    return (light + 0.05) / (dark + 0.05);
}

namespace ThemeUtils {
    QFileSelector m_fileSelector;
    void SetLightIconSet()
    {
        QStringList extraSelectors;
        m_fileSelector.setExtraSelectors(extraSelectors);
    }
    void SetDarkIconSet()
    {
        QStringList extraSelectors;
        extraSelectors << "darktheme";
        m_fileSelector.setExtraSelectors(extraSelectors);
    }

    QString GetThemedIcon(const QString& path)
    {
        if (m_fileSelector.extraSelectors().empty())
        {
            SetLightIconSet();
        }
        return m_fileSelector.select(path);
    }
}
