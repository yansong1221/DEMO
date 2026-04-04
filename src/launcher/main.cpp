#include "MainWindow.h"

#include <QApplication>
#include <cppmicroservices/FrameworkFactory.h>
#include <windows.h>
#include <QTranslator>

int
main(int argc, char** argv)
{
     SetConsoleOutputCP(CP_UTF8);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QApplication app(argc, argv);

    KDDockWidgets::initFrontend(KDDockWidgets::FrontendType::QtWidgets);

    QApplication::setApplicationName(QStringLiteral("CppMicroServicesHost"));

    QTranslator translator;

    // 加载 qm 文件
    if (translator.load("app_zh_CN.qm", "./translations")) // 或者文件路径
    {
        app.installTranslator(&translator);
    }


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
