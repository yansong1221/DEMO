#include "Config.h"
#include "imgui.h"
#include "imgui_stdlib.h"

void
Config::draw()
{
    ImGui::InputText("IP地址", &ip);
    ImGui::InputInt("端口号", &port);
    ImGui::InputInt("检测超时时间(毫秒)", &detect_timeout_ms);
}

void
Config::save(YAML::Node& conf) const
{
    conf["ip"] = ip;
    conf["port"] = port;
    conf["detect_timeout_ms"] = detect_timeout_ms;
}

void
Config::restore(YAML::Node conf)
{
    ip = conf["ip"].as<std::string>(ip);
    port = conf["port"].as<int>(port);
    detect_timeout_ms = conf["detect_timeout_ms"].as<int>(detect_timeout_ms);
}
