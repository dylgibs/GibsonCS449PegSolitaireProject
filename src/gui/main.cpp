#include <QApplication>

#include "mainwindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("PegSolitaire");
    QCoreApplication::setApplicationName("PegSolitaire");
    MainWindow w;
    w.show();
    return app.exec();
}
