#include "Service.h"
#include <boost/asio/dispatch.hpp>

Service::Service()
{
}

std::string Service::name() const
{
    return "juki";
}

bool Service::onThreadRun()
{
    ioc_->run();
    return false;
}

bool Service::onThreadStart(std::shared_ptr<IBasicConfig> config)
{
    ioc_ = std::make_unique<boost::asio::io_context>();
    workGuard_ =
        std::make_unique<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>>(
            boost::asio::make_work_guard(*ioc_));
    return true;
}

void Service::onThreadEnd()
{
    ioc_->stop();
    workGuard_.reset();
    ioc_.reset();
}

std::shared_ptr<service::ITaskService::IBasicConfig> Service::createConfig() const
{
    return std::make_shared<DemoTaskConfig>();
}

void Service::requestStop()
{
    if (!ioc_)
        return;

    boost::asio::dispatch(ioc_->get_executor(),
                          [this, self = shared_from_this()]() { workGuard_.reset(); });
}
