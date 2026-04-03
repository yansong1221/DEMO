#include "Service.h"
#include "AOIEvent.h"
#include "Config.h"
#include <boost/asio/post.hpp>

Service::Service(cppmicroservices::BundleContext const& context) : aiAgentTracker_(context)
{
    aiAgentTracker_.Open();
}

Service::~Service() { aiAgentTracker_.Close(); }

std::string
Service::displayName() const
{
    return "juki-aoi";
}

std::string
Service::name() const
{
    return "juki-aoi";
}

bool
Service::onThreadRun()
{
    ioc_->run();
    return false;
}

bool
Service::onThreadStart(std::shared_ptr<IBasicConfig> config)
{
    selfConfig_ = std::dynamic_pointer_cast<Config>(config);
    ioc_ = std::make_unique<boost::asio::io_context>();

    for (auto const& item : selfConfig_->aoiConfigs.items<Config::AoiConfig>())
    {
        auto task = std::make_shared<AOIEvent>(ioc_->get_executor(), aiAgentTracker_, item);
        task->start();
        tasks_.push_back(task);
    }
    return true;
}

void
Service::onThreadEnd()
{
    tasks_.clear();
    ioc_.reset();
    return;
}

std::shared_ptr<service::ITaskService::IBasicConfig>
Service::createConfig() const
{
    return std::make_shared<Config>();
}
void
Service::requestStop()
{
    if (!ioc_)
    {
        return;
    }

    boost::asio::post(*ioc_,
                      [this]()
                      {
                          for (auto const& task : tasks_)
                          {

                              task->stop();
                          }
                      });
    return;
}