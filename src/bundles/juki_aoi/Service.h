#pragma once
#include "cppmicroservices/ServiceTracker.h"
#include "imgui.h"
#include "service/IAIAgentService.h"
#include "service/ITaskService.h"

class DemoTaskConfig : public service::ITaskService::IBasicConfig
{
  public:
    void
    draw() override
    {
        ImGui::BulletText("显示中文");
        ImGui::BulletText("heelo");
        ImGui::Button("123456");
        ImGui::ShowDemoWindow();
    }

    void
    save(YAML::Node& conf) const override
    {
        conf["task_name"] = taskName;
        conf["interval_ms"] = intervalMs;
        conf["auto_restart"] = autoRestart;
    }

    void
    restore(YAML::Node conf) override
    {
        if (conf["task_name"])
        {
            taskName = conf["task_name"].as<std::string>();
        }
        if (conf["interval_ms"])
        {
            intervalMs = conf["interval_ms"].as<int>();
        }
        if (conf["auto_restart"])
        {
            autoRestart = conf["auto_restart"].as<bool>();
        }
    }

    std::string taskName = "demo.task";
    int intervalMs = 1000;
    bool autoRestart = false;
};

class Service : public service::ITaskService
{
  public:
    Service(cppmicroservices::BundleContext const& context);
    ~Service();

  public:
    std::string name() const override;

    bool onThreadRun() override;

    bool onThreadStart(std::shared_ptr<IBasicConfig> config) override;

    void onThreadEnd() override;

    std::shared_ptr<IBasicConfig> createConfig() const override;

    void requestStop() override;

  private:
    cppmicroservices::ServiceTracker<service::IAIAgentService> m_aiAgentTracker;
};