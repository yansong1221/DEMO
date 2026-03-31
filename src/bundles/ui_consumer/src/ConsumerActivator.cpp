#include "service/IWidgetService.h"
#include <QCoreApplication>
#include <QLabel>
#include <cppmicroservices/BundleActivator.h>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/ServiceTracker.h>
#include <iostream>

using namespace cppmicroservices;


class UiImpl : public service::IWidgetPlus
{
public:
    UiImpl()
    {
        w_ = new QLabel("Hello from the widget service!");
        w_->setWindowTitle("Hello");
    }
    ~UiImpl()
    {
        std::cout << "UiImpl destructor" << std::endl;
        delete w_;
    }
    QWidget* widget() override { return w_; }
    // void destroyWidget(QWidget* widget) override { delete static_cast<QLabel*>(widget); }

    void hello() override { throw std::logic_error("The method or operation is not implemented."); }

    QString uniqueName() const override { return "demo.widget"; }

protected:
    QLabel* w_;
};

namespace demo {
class ConsumerActivator : public BundleActivator
{
    // std::unique_ptr<ServiceTracker<demo::IGreetingService>> m_tracker;
public:
    void Start(BundleContext context) override
    {
        m_service = std::make_shared<UiImpl>();
        m_reg     = context.RegisterService<service::IWidgetService>(m_service);
    }
    void Stop(BundleContext) override
    {
        m_reg.Unregister();
        m_service.reset();
    }

private:
    cppmicroservices::ServiceRegistration<UiImpl> m_reg;
    std::shared_ptr<UiImpl> m_service;

};
} // namespace demo
CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(demo::ConsumerActivator)
