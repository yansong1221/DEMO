#pragma once
#include "service/IAIAgentService.h"

class Panel
{
  public:
    std::shared_ptr<service::IAIAgentService::IDetectPanel> detectPanel;

  public:
    static std::shared_ptr<Panel> loadFromFile(std::filesystem::path const& xmlPath,
                                               std::shared_ptr<service::IAIAgentService::IDetectPanel> panel);
};