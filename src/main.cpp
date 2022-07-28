#include <QApplication>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(resources);

    QApplication app(argc, argv);
    app.setOrganizationName("Tableau");
    app.setApplicationName("TLV");
    app.setApplicationVersion(APP_VERSION);

    std::unique_ptr<MainWindow> mainWin = std::make_unique<MainWindow>();
    mainWin->show();
    return app.exec();
}
