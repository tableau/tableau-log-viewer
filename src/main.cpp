#include <QApplication>

#include "mainwindow.h"
#include <QVector>

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(resources);

    QApplication app(argc, argv);
    app.setOrganizationName("Tableau");
    app.setApplicationName("TLV");
    app.setApplicationVersion(APP_VERSION);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);

    QStringList args = app.arguments();

    std::unique_ptr<MainWindow> mainWin = std::make_unique<MainWindow>(args);
    mainWin->show();
    return app.exec();
}
