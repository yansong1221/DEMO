#include "MainWindow.h"

#include <QApplication>

int main(int argc, char** argv)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QApplication app(argc, argv);

    KDDockWidgets::initFrontend(KDDockWidgets::FrontendType::QtWidgets);

    QApplication::setApplicationName(QStringLiteral("CppMicroServicesHost"));

    MainWindow w;
    w.resize(1024, 768);
    w.show();
    return app.exec();
}
