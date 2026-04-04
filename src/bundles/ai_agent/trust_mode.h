#pragma once
#include "imgui.h"
#include "imgui_extend/table_view.h"
#include "service/IAIAgentService.h"
#include "service/IUIService.h"
#include <QDateTime>
#include <QObject>
#include <SQLiteCpp/SQLiteCpp.h>
#include <yaml-cpp/yaml.h>

class ProgramTrustModeDrawer
    : public QObject
    , public ImGui::extend::TableView
    , public service::ITrustProgramService
    , public service::IImGuiDrawService
{
    Q_OBJECT
  public:
    struct program_info
    {
        int64_t id;
        std::string line;
        std::string station;
        std::string program;
        bool trust = false;
        QDateTime create_date;
        QDateTime update_date;

        bool modified = false;
    };

    enum class col_type : int
    {
        trust_mode = 0,
        id,
        line,
        station,
        program,
        create_date,
        update_date
    };

    enum class display_trust_type
    {
        all,
        trust,
        no_trust,
    };

    bool isTrust(std::string const& line, std::string const& station, std::string const& name) override;
    void drawImGui() override;

    int columnCount() const override;
    std::string headerLable(int column) const override;
    void drawCell(int row, int column) override;
    int rowCount() const override;

    ImGuiTableColumnFlags headerFlags(int column) const override;
    int rowCompare(int left_row, int right_row, int column) const override;
    bool rowPassFilter(int row) const override;

  public:
    bool is_trust_unsafe(std::string_view line, std::string_view station, std::string_view program);

  public:
    void draw(ImVec2 const& size = ImVec2(0, 0)) override;
    void refresh_db();
    void apply_db();

  private:
    void set_trust_all(bool trust);
    static std::string make_cache_key(std::string_view line, std::string_view station, std::string_view program);
    void clear();

    std::string display_trust_type_string(ProgramTrustModeDrawer::display_trust_type& type);

  private:
    std::unique_ptr<SQLite::Database> db_;
    std::vector<program_info> program_list_;
    std::unordered_map<std::string, int> cache_index_;

    ImGuiTextFilter textFilter_;

    display_trust_type display_trust_type_ = display_trust_type::all;

    std::chrono::steady_clock::time_point last_update_time_;
};
