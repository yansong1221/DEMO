#include "demo/GreetingActivator.h"
#include "common/Logger.h"

#include <cppmicroservices/ServiceProperties.h>

#include <chrono>
#include <iostream>
#include <thread>

#include "imgui.h"

using namespace cppmicroservices;

namespace demo
{

    class DemoTaskConfig : public ITaskService::IBasicConfig
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

    class GreetingTaskService : public ITaskService
    {
      public:
        ~GreetingTaskService() { std::cout << "[TaskService] destructor" << std::endl; }

        // ITaskService 接口实现
        bool
        onThreadStart(std::shared_ptr<IBasicConfig> config) override
        {
            auto typedConfig = std::dynamic_pointer_cast<DemoTaskConfig>(config);
            if (!typedConfig)
            {
                typedConfig = std::make_shared<DemoTaskConfig>();
                if (config)
                {
                    YAML::Node conf;
                    config->save(conf);
                    typedConfig->restore(conf);
                }
            }

            m_lastConfig = typedConfig;
            m_running = true;
            m_counter = 0;

            std::cout << "[TaskService] onThreadStart"
                      << " task_name=" << typedConfig->taskName << " interval_ms=" << typedConfig->intervalMs
                      << " auto_restart=" << std::boolalpha << typedConfig->autoRestart << std::endl;
            return true;
        }

        bool
        onThreadRun() override
        {
            if (!m_running)
            {
                return false;
            }

            // 模拟任务执行
            m_counter++;
            if (m_counter % 10 == 0)
            {
                common::Log::info(std::format("[TaskService] running... count= {}", m_counter));
            }

            // 模拟工作间隔
            std::this_thread::sleep_for(std::chrono::milliseconds(m_lastConfig ? m_lastConfig->intervalMs : 1000));

            return true;
        }

        void
        onThreadEnd() override
        {
            m_running = false;
            std::cout << "[TaskService] onThreadEnd"
                      << " task_name=" << (m_lastConfig ? m_lastConfig->taskName : std::string("demo.task"))
                      << " final_count=" << m_counter << std::endl;
        }

        std::shared_ptr<IBasicConfig>
        createConfig() const override
        {
            auto config = std::make_shared<DemoTaskConfig>();
            if (m_lastConfig)
            {
                *config = *m_lastConfig;
            }
            return config;
        }

        std::string
        name() const override
        {
            return "dome.task";
        }

        void
        requestStop() override
        {
            throw std::logic_error("The method or operation is not implemented.");
        }

      private:
        bool m_running = false;
        int m_counter = 0;
        std::shared_ptr<DemoTaskConfig> m_lastConfig = std::make_shared<DemoTaskConfig>();
    };

    void
    GreetingActivator::Start(BundleContext context)
    {
        common::Log::init(context);

        m_service = std::make_shared<GreetingTaskService>();

        ServiceProperties props;
        props["service.description"] = std::string("Demo ITaskService with YAML configuration");
        props["task.service.kind"] = std::string("demo");

        m_reg = context.RegisterService<ITaskService>(m_service, props);

        // 使用 Logger 输出日志
        common::Log::info("GreetingActivator started and ITaskService registered.");

        std::cout << "GreetingActivator::Start" << std::endl;
    }

    void
    GreetingActivator::Stop(BundleContext)
    {
        m_reg.Unregister();
        m_service.reset();

        std::cout << "GreetingActivator::Stop" << std::endl;

        common::Log::reset();
    }

} // namespace demo

CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(demo::GreetingActivator)
