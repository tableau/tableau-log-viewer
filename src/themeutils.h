#pragma once

class QColor;
class QString;
class QStringList;

namespace ThemeUtils {
    QStringList GetThemeNames();
    void SwitchTheme(const QString& themeName, QWidget* widget);
    double Luminance(QColor color);
    double ContrastRatio(QColor a, QColor b);
    void SetLightIconSet();
    void SetDarkIconSet();
    QString GetThemedIcon(const QString& path);
}
