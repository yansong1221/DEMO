#pragma once

#include "ITaskService.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace service
{

    class ITaskServiceManager
    {
      public:
        class ITaskServiceController
        {
          public:
            enum class TaskServiceStatus
            {
                Running,
                Stopped,
            };
            using TaskServiceStatusCallback = std::function<void(TaskServiceStatus)>;

            virtual ~ITaskServiceController() = default;

            virtual std::string symbolicName() const = 0;
            virtual std::string serviceName() const = 0;
            virtual std::string displayServiceName() const = 0;
            virtual TaskServiceStatus status() const = 0;
            virtual std::shared_ptr<ITaskService::IBasicConfig> createConfig() const = 0;
            virtual bool start(std::shared_ptr<ITaskService::IBasicConfig> config) = 0;
            virtual void stop() = 0;
            virtual void setStatusCallback(TaskServiceStatusCallback callback) = 0;
        };

        using ControllerPtr = std::shared_ptr<ITaskServiceController>;

        enum class ControllerEvent
        {
            Registered,
            Unregistered,
        };

        using ControllerEventCallback = std::function<void(ControllerPtr, ControllerEvent)>;

        virtual ~ITaskServiceManager() = default;

        virtual std::vector<ControllerPtr> listTaskControllers() const = 0;
        virtual void setControllerEventCallback(ControllerEventCallback callback) = 0;
    };

} // namespace service
