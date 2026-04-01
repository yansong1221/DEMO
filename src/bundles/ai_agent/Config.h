
#pragma once
#include "service/ITaskService.h"

class Config : public service::ITaskService::IBasicConfig
{
public:
    void draw(ImGuiContext* ctx) override;
    void save(YAML::Node& conf) const override;
    void restore(YAML::Node conf) override;

    std::string ip        = "127.0.0.1";
    int port              = 7783;
    int detect_timeout_ms = 5000;
};
