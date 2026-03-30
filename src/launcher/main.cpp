#include "MainWindow.h"

#include <QApplication>
#include <cppmicroservices/FrameworkFactory.h>

int main(int argc, char** argv)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QApplication app(argc, argv);

    KDDockWidgets::initFrontend(KDDockWidgets::FrontendType::QtWidgets);

    QApplication::setApplicationName(QStringLiteral("CppMicroServicesHost"));

    cppmicroservices::FrameworkFactory factory;
    auto framework = factory.NewFramework();
    framework.Init();
    framework.Start();

    MainWindow w(framework.GetBundleContext());
    w.resize(1024, 768);
    w.show();

    int ret = app.exec();

    framework.Stop();
    framework.WaitForStop((std::chrono::milliseconds::max)());

    return ret;
}
