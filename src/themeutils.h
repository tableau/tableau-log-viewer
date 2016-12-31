#pragma once

namespace ThemeUtils {
    void SwitchTheme(const QString& themeName, QWidget* widget);
    double Luminance(QColor color);
    double ContrastRatio(QColor a, QColor b);
}
