#pragma once
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace service
{
    class ITaskService
    {
      public:
        struct IBasicConfig
        {
            virtual ~IBasicConfig() = default;
            virtual void draw() = 0;
            virtual void save(YAML::Node& conf) const = 0;
            virtual void restore(YAML::Node conf) = 0;
        };

        virtual ~ITaskService() = default;

        virtual std::string name() const = 0;
        virtual std::string
        displayName() const
        { return name(); };

        virtual void requestStop() = 0;
        virtual bool onThreadRun() = 0;
        virtual bool onThreadStart(std::shared_ptr<IBasicConfig> config) = 0;
        virtual void onThreadEnd() = 0;

        virtual std::shared_ptr<IBasicConfig> createConfig() const = 0;
    };
} // namespace service
