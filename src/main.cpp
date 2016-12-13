#include <QApplication>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(resources);

    QApplication app(argc, argv);
    app.setOrganizationName("Tableau");
    app.setApplicationName("TLV");
    app.setApplicationVersion(APP_VERSION);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);

    app.setStyle(QStyleFactory::create("Fusion"));

    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53,53,53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25,25,25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53,53,53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53,53,53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));

    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);

    // Solarized Dark
    QPalette solDarkPalette;
    solDarkPalette.setColor(QPalette::Window,        QColor(0x001e26));
    solDarkPalette.setColor(QPalette::WindowText,    QColor(0x839496));
    solDarkPalette.setColor(QPalette::Base,          QColor(0x002b36));
    solDarkPalette.setColor(QPalette::AlternateBase, QColor(0x073642));
    solDarkPalette.setColor(QPalette::ToolTipBase,   QColor(0x839496));
    solDarkPalette.setColor(QPalette::ToolTipText,   QColor(0x839496));
    solDarkPalette.setColor(QPalette::Text,          QColor(0x839496));
    solDarkPalette.setColor(QPalette::Button,        QColor(0x001e26));
    solDarkPalette.setColor(QPalette::ButtonText,    QColor(0x839496));
    solDarkPalette.setColor(QPalette::BrightText,    Qt::red);
    solDarkPalette.setColor(QPalette::Link,          QColor(0x2aa198));

    solDarkPalette.setColor(QPalette::Highlight, QColor(0x268bd2));
    solDarkPalette.setColor(QPalette::HighlightedText, Qt::black);


    solDarkPalette.setColor(QPalette::Disabled, QPalette::Text, QColor(0x586e75));
    solDarkPalette.setColor(QPalette::Disabled, QPalette::Button, QColor(0x586e75));
    solDarkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(0x073642));
    solDarkPalette.setColor(QPalette::Disabled, QPalette::Base, QColor(0x001e26));

    solDarkPalette.setColor(QPalette::Light, QColor(0x073642));

    // Solarized Light
    QPalette solPalette;
    solPalette.setColor(QPalette::Window,        QColor(0xe6ddc1));
    solPalette.setColor(QPalette::WindowText,    QColor(0x657b83));
    solPalette.setColor(QPalette::Base,          QColor(0xfdf6e3));
    solPalette.setColor(QPalette::AlternateBase, QColor(0xeee8d5));
    solPalette.setColor(QPalette::ToolTipBase,   QColor(0x657b83));
    solPalette.setColor(QPalette::ToolTipText,   QColor(0x657b83));
    solPalette.setColor(QPalette::Text,          QColor(0x657b83));
    solPalette.setColor(QPalette::Button,        QColor(0xe6ddc1));
    solPalette.setColor(QPalette::ButtonText,    QColor(0x657b83));
    solPalette.setColor(QPalette::BrightText,    Qt::red);
    solPalette.setColor(QPalette::Link,          QColor(0x2aa198));

    solPalette.setColor(QPalette::Highlight, QColor(0x268bd2));
    solPalette.setColor(QPalette::HighlightedText, Qt::black);


    solPalette.setColor(QPalette::Disabled, QPalette::Text, QColor(0x93a1a1));
    solPalette.setColor(QPalette::Disabled, QPalette::Button, QColor(0x93a1a1));
    solPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(0xeee8d5));
    solPalette.setColor(QPalette::Disabled, QPalette::Base, QColor(0xe6ddc1));

    solPalette.setColor(QPalette::Light, QColor(0xeee8d5));

    app.setPalette(solDarkPalette);

    std::unique_ptr<MainWindow> mainWin = std::make_unique<MainWindow>();
    mainWin->show();
    return app.exec();
}
