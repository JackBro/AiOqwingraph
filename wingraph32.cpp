#include <QApplication>

#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setOrganizationName("Hex-Rays");
    app.setApplicationName("WinGraph32");
    MainWindow mainWin;
    mainWin.show();
    return app.exec();
}

