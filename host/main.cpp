#include "MainWindow.h"

#include <QApplication>

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("CppMicroServicesHost"));
    MainWindow w;
    w.show();
    return app.exec();
}
