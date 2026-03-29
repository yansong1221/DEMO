#pragma once
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

struct ImGuiContext;

namespace spdlog {
class logger;
}

namespace service {
class ITaskService
{
public:
    struct IBasicConfig
    {
        virtual ~IBasicConfig()                   = default;
        virtual void draw(ImGuiContext* ctx)      = 0;
        virtual void save(YAML::Node& conf) const = 0;
        virtual void restore(YAML::Node conf)     = 0;
        virtual std::optional<std::string> displayName() const { return std::nullopt; }
    };

    virtual ~ITaskService() = default;

    virtual std::string name() const = 0;

    virtual bool onThreadRun()                                       = 0;
    virtual bool onThreadStart(std::shared_ptr<IBasicConfig> config) = 0;
    virtual void onThreadEnd()                                       = 0;

    virtual void setLanguage(std::string const& language)          = 0;
    virtual std::shared_ptr<IBasicConfig> createYamlConfig() const = 0;

    // 设置日志记录器，使服务能够直接输出日志
    virtual void setLogger(std::shared_ptr<spdlog::logger> logger) = 0;
};
} // namespace service
