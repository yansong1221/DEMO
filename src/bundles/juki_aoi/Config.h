#pragma once
#include "common/misc.h"
#include "imgui_extend/component.h"
#include "imgui_extend/basic_config.h"
#include "service/ITaskService.h"
#include <filesystem>

class Config : public ImGui::extend::GroupBasicConfig
{
  public:
    class AoiConfig : public ImGui::extend::BasicConfig
    {
      public:
        std::string
        displayName() const override
        {
            return "AOI";
        }
        void
        draw() override
        {
            ImGui::extend::InputDirectory("AOI Directory", &aoi_dir);
        }
        void
        save(YAML::Node& conf) const override
        {
            conf["aoi_dir"] = aoi_dir;
        }
        void
        restore(YAML::Node conf) override
        {
            using namespace std::filesystem;
            aoi_dir = conf["aoi_dir"].as<std::string>("");
        }
        std::string aoi_dir = "aoi";
    };
    Config();

  protected:
    std::vector<Item> prepareItems() override;

  private:
    class AoiConfigs : public ImGui::extend::ArrayBasicConfig
    {
        using ArrayBasicConfig::ArrayBasicConfig;

      public:
        std::string
        displayName() const override
        {
            return "AOI Configs";
        }
    };
    AoiConfigs m_aoiConfigs_;
};
