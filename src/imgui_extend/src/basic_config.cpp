#include "imgui_extend/basic_config.h"
#include "imgui.h"
#include "imgui_extend/component.h"
#include <QCoreApplication>
#include <ranges>

namespace ImGui::extend
{
    bool
    TrustModeConfig::isTrust(std::shared_ptr<service::IAIAgentService> serv,
                             std::string const& line,
                             std::string const& station,
                             std::string const& program) const
    {
        bool program_trust = false;
        if (serv)
        {
            program_trust = serv->isTrustProgram(line, station, program);
        }

        switch (trust_mode_)
        {
            case TrustModeConfig::mode::off:
                break;
            case TrustModeConfig::mode::on:
                return true;
                break;
            case TrustModeConfig::mode::with_program:
                return program_trust;
                break;
            default:
                break;
        }
        return false;
    }

    static std::string
    mode_to_string(TrustModeConfig::mode m)
    {
        switch (m)
        {
            case TrustModeConfig::mode::off:
                return QCoreApplication::translate("imgui::extend", "OFF").toStdString();
            case TrustModeConfig::mode::on:
                return QCoreApplication::translate("imgui::extend", "ON").toStdString();
            case TrustModeConfig::mode::with_program:
                return QCoreApplication::translate("imgui::extend", "Trust by program").toStdString();
            default:
                return QCoreApplication::translate("imgui::extend", "Unknown mode").toStdString();
        }
    }

    void
    TrustModeConfig::draw()
    {
        if (ImGui::BeginCombo(QCoreApplication::translate("imgui::extend", "Trust Mode").toUtf8(),
                              mode_to_string(trust_mode_).c_str()))
        {
            if (ImGui::Selectable(QCoreApplication::translate("imgui::extend", "OFF").toUtf8(),
                                  trust_mode_ == mode::off))
            {
                trust_mode_ = mode::off;
            }
            if (ImGui::Selectable(QCoreApplication::translate("imgui::extend", "ON").toUtf8(), trust_mode_ == mode::on))
            {
                trust_mode_ = mode::on;
            }
            if (ImGui::Selectable(QCoreApplication::translate("imgui::extend", "Trust by program").toUtf8(),
                                  trust_mode_ == mode::with_program))
            {
                trust_mode_ = mode::with_program;
            }
            ImGui::EndCombo();
        }
    }

    void
    TrustModeConfig::restore(YAML::Node item)
    {
        trust_mode_ = (TrustModeConfig::mode)item["mode"].as<int>((int)trust_mode_);
    }

    void
    TrustModeConfig::save(YAML::Node& conf) const
    {
        conf["mode"] = (int)trust_mode_;
    }

    std::string
    TrustModeConfig::displayName() const
    {
        return QCoreApplication::translate("imgui::extend", "Trust Mode Config").toStdString();
    }

    std::string
    TrustModeConfig::configKey() const
    {
        return "TrustMode";
    }

    void
    ArrayBasicConfig::draw()
    {
        for (int i = 0; i < m_items.size(); i++)
        {
            auto& info = m_items[i];
            auto d_name = info->displayName();
            auto label = QCoreApplication::translate("imgui::extend", "Config%1").arg(i + 1);

            ImGui::PushID(i);

            if (!d_name.empty())
            {
                label += QString::fromStdString(std::format("({})", d_name));
            }
            if (ImGui::TreeNodeEx("config", ImGuiTreeNodeFlags_DefaultOpen, label.toUtf8()))
            {
                ImGui::BeginGroup();
                info->draw();
                ImGui::EndGroup();

                ImGui::Separator();

                if (ImGui::extend::DeleteButton(
                        QCoreApplication::translate("imgui::extend", "Delete-[%1]").arg(label).toUtf8()))
                {
                    m_items.erase(m_items.begin() + i);
                    i--;
                }
                ImGui::TreePop();
            }
            ImGui::Separator();
            ImGui::PopID();
        }
        if (ImGui::Button(QCoreApplication::translate("imgui::extend", "Add").toUtf8()))
        {
            m_items.push_back(newConfigItem());
        }
    }

    void
    ArrayBasicConfig::save(YAML::Node& conf) const
    {
        conf.reset();
        for (auto const& confPtr : m_items)
        {
            YAML::Node yaml;
            confPtr->save(yaml);
            conf.push_back(yaml);
        }
    }

    void
    ArrayBasicConfig::restore(YAML::Node conf)
    {
        m_items.clear();
        for (auto item : conf)
        {
            auto confPtr = newConfigItem();
            confPtr->restore(item);
            m_items.push_back(confPtr);
        }
    }

    void
    GroupBasicConfig::draw()
    {
        auto items = prepareItems();
        auto visable_items = items | std::views::filter([](Item const& v) { return v.visable; });

        ImGui::PushID(this);

        int index = 0;
        for (auto& item : visable_items | std::views::filter([](Item const& v) { return v.view != Item::tab; }))
        {
            ImGui::PushID(index++);
            switch (item.view)
            {
                case Item::tree:
                {
                    if (ImGui::TreeNodeEx(item.conf->displayName().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        item.conf->draw();
                        ImGui::TreePop();
                    }
                }
                break;
                default:
                {
                    ImGui::SeparatorText(item.conf->displayName().c_str());
                    ImGui::Indent();
                    item.conf->draw();
                    ImGui::Unindent();
                }
                break;
            }
            ImGui::PopID();
        }

        if (std::ranges::find_if(visable_items, [](Item const& v) { return v.view == Item::tab; })
            != visable_items.end())
        {
            ImGui::BeginTabBar("##TAB");
            for (auto& item : visable_items | std::views::filter([](Item const& v) { return v.view == Item::tab; }))
            {
                ImGui::PushID(index++);

                if (ImGui::BeginTabItem(item.conf->displayName().c_str()))
                {
                    item.conf->draw();
                    ImGui::EndTabItem();
                }

                ImGui::PopID();
            }
            ImGui::EndTabBar();
        }
        ImGui::PopID();
    }

    void
    GroupBasicConfig::save(YAML::Node& conf) const
    {
        for (auto const& item : prepareItems())
        {
            YAML::Node yaml;
            item.conf->save(yaml);
            conf[item.conf->configKey()] = yaml;
        }
    }

    void
    GroupBasicConfig::restore(YAML::Node conf)
    {
        for (auto& item : prepareItems())
        {
            YAML::Node yaml = conf[item.conf->configKey()];
            item.conf->restore(yaml);
        }
    }

    void
    TableArrayBasicConfig::draw()
    {
        if (ImGui::Button(QCoreApplication::translate("imgui::extend", "Add").toUtf8()))
        {
            m_items.push_back(newConfigItem());
            tableReloadRows();
        }
        TableView::draw(ImVec2(0.0f, 300.f));
    }

    void
    TableArrayBasicConfig::restore(YAML::Node item)
    {
        ArrayBasicConfig::restore(item);
        tableReloadRows();
    }

    int
    TableArrayBasicConfig::columnCount() const
    {
        return 3;
    }

    std::string
    TableArrayBasicConfig::headerLable(int column) const
    {
        switch (column)
        {
            case 0:
                return QCoreApplication::translate("imgui::extend", "Index").toStdString();
                break;
            case 1:
                return QCoreApplication::translate("imgui::extend", "Config").toStdString();
                break;
            case 2:
                return QCoreApplication::translate("imgui::extend", "OP").toStdString();
                break;
            default:
                break;
        }
        return {};
    }

    int
    TableArrayBasicConfig::rowCount() const
    {
        return m_items.size();
    }

    void
    TableArrayBasicConfig::drawCell(int row, int column)
    {
        if (row >= m_items.size())
        {
            return;
        }

        auto& info = m_items[row];

        switch (column)
        {
            case 0:
            {
                ImGui::Text("%d", row + 1);
            }
            break;
            case 1:
            {
                info->draw();
            }
            break;
            case 2:
            {
                if (DeleteButton())
                {
                    m_items.erase(m_items.begin() + row);
                    tableReloadRows();
                }
            }
            break;
            default:
                break;
        }
    }

    ImGuiTableColumnFlags
    TableArrayBasicConfig::headerFlags(int column) const
    {
        ImGuiTableColumnFlags flags = TableView::headerFlags(column);
        flags |= ImGuiTableColumnFlags_NoSort;
        switch (column)
        {
            case 1:
                flags |= ImGuiTableColumnFlags_WidthStretch;
                break;
            default:
                break;
        }
        return flags;
    }

    std::vector<ImGui::extend::GroupBasicConfig::Item>
    CustomAngleConfig::prepareItems()
    {
        return {
            { &program_list_ },
            { &station_list_ },
        };
    }

    void
    CustomAngleConfig::draw()
    {
        ImGui::Checkbox(QCoreApplication::translate(
                            "imgui::extend",
                            R"(Is angle detection mandatory? If no suitable angle is available, skip detection)")
                            .toUtf8(),
                        &required_);
        GroupBasicConfig::draw();
    }

    void
    CustomAngleConfig::restore(YAML::Node item)
    {
        required_ = item["required"].as<bool>(false);
        GroupBasicConfig::restore(item);
    }

    void
    CustomAngleConfig::save(YAML::Node& conf) const
    {
        conf["required"] = required_;
        GroupBasicConfig::save(conf);
    }

    std::optional<int>
    CustomAngleConfig::fixedAngle(std::string_view line,
                                  std::string_view station,
                                  std::string_view program,
                                  int board_index) const
    {
        auto find_boards_angle = [&](auto const& config_boards)
        {
            auto iter = std::ranges::find_if(config_boards,
                                             [&](auto const& info) { return info->board_index == board_index; });
            if (iter == config_boards.end())
            {
                return 0;
            }

            return (*iter)->angle;
        };
        {
            auto items = program_list_.items<ProgramAngleConfig>();
            auto iter = std::ranges::find_if(items, [&](auto const& angle) { return angle->name == program; });
            if (iter != items.end())
            {
                return find_boards_angle((*iter)->boards.items<BoardInfoConfig>());
            }
        }
        {
            auto items = station_list_.items<StationAngleConfig>();
            auto iter = std::ranges::find_if(items,
                                             [&](auto const& angle)
                                             { return angle->line == line && angle->station == station; });
            if (iter != items.end())
            {
                return find_boards_angle((*iter)->boards.items<BoardInfoConfig>());
            }
        }
        return std::nullopt;
    }

    bool
    CustomAngleConfig::isRequired() const
    {
        return required_;
    }

    std::optional<int>
    CustomAngleConfig::fixedAngleWithOriginalAngle(std::string_view line,
                                                   std::string_view station,
                                                   std::string_view program,
                                                   int board_index,
                                                   int original_angle) const
    {
        auto newAngle = fixedAngle(line, station, program, board_index);
        if (!newAngle && isRequired())
        {
            return std::nullopt;
        }
        return newAngle ? newAngle.value() : original_angle;
    }

    std::string
    CustomAngleConfig::displayName() const
    {
        return QCoreApplication::translate("imgui::extend", "Custom Angle Config").toStdString();
    }

    std::string
    CustomAngleConfig::configKey() const
    {
        return "CustomAngle";
    }

    void
    CustomAngleConfig::BoardInfoConfig::draw()
    {
        InputInt(QCoreApplication::translate("imgui::extend", "Board Index").toUtf8(), &board_index, 0);
        InputInt(QCoreApplication::translate("imgui::extend", "Angle").toUtf8(), &angle, 0);
    }

    void
    CustomAngleConfig::BoardInfoConfig::save(YAML::Node& conf) const
    {
        conf["board_index"] = board_index;
        conf["angle"] = angle;
    }

    void
    CustomAngleConfig::BoardInfoConfig::restore(YAML::Node item)
    {
        board_index = item["board_index"].as<int>(board_index);
        angle = item["angle"].as<int>(angle);
    }

    std::string
    CustomAngleConfig::BoardInfoConfig::displayName() const
    {
        return QCoreApplication::translate("imgui::extend", "Board Info").toStdString();
    }

    std::string
    CustomAngleConfig::BoardInfoConfig::configKey() const
    {
        return "BoardInfo";
    }

    std::vector<ImGui::extend::GroupBasicConfig::Item>
    CustomAngleConfig::ProgramAngleConfig::prepareItems()
    {
        return { { &boards } };
    }

    std::string
    CustomAngleConfig::ProgramAngleConfig::displayName() const
    {
        return QCoreApplication::translate("imgui::extend", "Program Angle Config").toStdString();
    }

    void
    CustomAngleConfig::ProgramAngleConfig::draw()
    {
        ImGui::InputText(QCoreApplication::translate("imgui::extend", "Program Name").toUtf8(), &name);
        GroupBasicConfig::draw();
    }

    void
    CustomAngleConfig::ProgramAngleConfig::restore(YAML::Node item)
    {
        name = item["name"].as<std::string>("");
        GroupBasicConfig::restore(item);
    }

    void
    CustomAngleConfig::ProgramAngleConfig::save(YAML::Node& conf) const
    {
        conf["name"] = name;
        GroupBasicConfig::save(conf);
    }

    std::string
    CustomAngleConfig::ProgramAngleConfig::configKey() const
    {
        return "ProgramAngle";
    }

    std::vector<ImGui::extend::GroupBasicConfig::Item>
    CustomAngleConfig::StationAngleConfig::prepareItems()
    {
        return { { &boards } };
    }

    void
    CustomAngleConfig::StationAngleConfig::draw()
    {
        ImGui::InputText(QCoreApplication::translate("imgui::extend", "Line").toUtf8(), &line);
        ImGui::InputText(QCoreApplication::translate("imgui::extend", "Station").toUtf8(), &station);
        GroupBasicConfig::draw();
    }

    void
    CustomAngleConfig::StationAngleConfig::restore(YAML::Node item)
    {
        line = item["line"].as<std::string>("");
        station = item["station"].as<std::string>("");
        GroupBasicConfig::restore(item);
    }

    void
    CustomAngleConfig::StationAngleConfig::save(YAML::Node& conf) const
    {
        conf["line"] = line;
        conf["station"] = station;
        GroupBasicConfig::save(conf);
    }

    std::string
    CustomAngleConfig::StationAngleConfig::displayName() const
    {
        return QCoreApplication::translate("imgui::extend", "Station Angle Config").toStdString();
    }

    std::string
    CustomAngleConfig::StationAngleConfig::configKey() const
    {
        return "StationAngle";
    }

    std::string
    CustomAngleConfig::StationArrayBasicConfig::displayName() const
    {
        return QCoreApplication::translate("imgui::extend", "Custom station angle").toStdString();
    }

    std::string
    CustomAngleConfig::StationArrayBasicConfig::configKey() const
    {
        return "StationArray";
    }

    std::string
    CustomAngleConfig::ProgramArrayBasicConfig::displayName() const
    {
        return QCoreApplication::translate("imgui::extend", "Custom program angle").toStdString();
    }

    std::string
    CustomAngleConfig::ProgramArrayBasicConfig::configKey() const
    {
        return "ProgramArray";
    }

    std::string
    CustomAngleConfig::BoardInfoArrayConfig::displayName() const
    {
        return QCoreApplication::translate("imgui::extend", "Board Info Array").toStdString();
    }

    std::string
    CustomAngleConfig::BoardInfoArrayConfig::configKey() const
    {
        return "BoardInfoArray";
    }

} // namespace ImGui::extend