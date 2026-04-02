#pragma once
#include "service/ITaskService.h"
#include <filesystem>

class Config : public service::ITaskService::IBasicConfig
{
  public:
    void draw(ImGuiContext* ctx) override;
    void save(YAML::Node& conf) const override;
    void restore(YAML::Node conf) override;

    std::filesystem::path log_dir = "logs";
};
