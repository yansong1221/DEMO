#include "trust_mode.h"
#include "common/Logger.h"
#include "common/Misc.h"
#include "imgui_extend/component.h"
#include <QCoreApplication>
#include <QDir>
#include <QThread>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/io_context.hpp>
#include <format>
#include <future>

char const* g_init_sql = R"(

CREATE TABLE IF NOT EXISTS "program" (
  "id" INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  "line" TEXT NOT NULL,
  "station" TEXT NOT NULL,
  "program" TEXT NOT NULL,
  "trust" integer NOT NULL DEFAULT 0,
  "create_date" DATE NOT NULL DEFAULT (datetime('now', 'localtime')),
  "update_date" DATE NOT NULL DEFAULT (datetime('now', 'localtime'))
);

CREATE UNIQUE INDEX IF NOT EXISTS "_program_key"
ON "program" (
  "line" ASC,
  "station" ASC,
  "program" ASC
);

)";

namespace detail
{
    static std::unique_ptr<SQLite::Database>
    get_db()
    {
        auto db_path = QDir(QCoreApplication::applicationDirPath()).filePath("trust_program.db");
        try
        {
            auto db = std::make_unique<SQLite::Database>(db_path.toUtf8(),
                                                         SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE,
                                                         3000);
            db->exec(g_init_sql);
            return db;
        }
        catch (SQLite::Exception const& e)
        {
            common::Log::error("Failed to initialize program trust mode database: {} path: {}",
                               e.what(),
                               db_path.toStdString());
        }
        return nullptr;
    }
} // namespace detail

bool
ProgramTrustModeDrawer::isTrust(std::string const& line, std::string const& station, std::string const& name)
{
    if (QCoreApplication::instance()->thread()->currentThreadId() == QThread::currentThreadId())
    {
        return is_trust_unsafe(line, station, name);
    }
    std::promise<bool> promise;
    auto future = promise.get_future();

    QMetaObject::invokeMethod(
        this,
        [&]()
        {
            auto result = is_trust_unsafe(line, station, name);
            promise.set_value(result);
        },
        Qt::QueuedConnection);

    return future.get();
}

void
ProgramTrustModeDrawer::drawImGui()
{
    this->draw();
}

int
ProgramTrustModeDrawer::columnCount() const
{
    return 7;
}

std::string
ProgramTrustModeDrawer::headerLable(int column) const
{
    switch ((col_type)column)
    {
        case col_type::trust_mode:
            return tr("Trust Mode").toStdString();
        case col_type::id:
            return tr("ID").toStdString();
        case col_type::line:
            return tr("Line").toStdString();
        case col_type::station:
            return tr("Station").toStdString();
        case col_type::program:
            return tr("Program Name").toStdString();
        case col_type::create_date:
            return tr("Create Time").toStdString();
        case col_type::update_date:
            return tr("Latest Time").toStdString();
        default:
            break;
    }
    return std::format("Unknown-{}", column);
}

void
ProgramTrustModeDrawer::drawCell(int row, int column)
{
    auto& info = program_list_[row];
    switch ((col_type)column)
    {
        case col_type::trust_mode:
        {
            if (ImGui::Checkbox("##信任模式", &info.trust))
            {
                info.modified = true;
                apply_db();
            }
        }
        break;
        case col_type::id:
            ImGui::Text("%d", info.id);
            break;
        case col_type::line:
            ImGui::TextUnformatted(info.line.c_str());
            break;
        case col_type::station:
            ImGui::TextUnformatted(info.station.c_str());
            break;
        case col_type::program:
            ImGui::TextUnformatted(info.program.c_str());
            break;
        case col_type::create_date:
            ImGui::TextUnformatted(info.create_date.toString("yyyy-MM-dd hh:mm:ss").toUtf8());
            break;
        case col_type::update_date:
            ImGui::TextUnformatted(info.update_date.toString("yyyy-MM-dd hh:mm:ss").toUtf8());
            break;
        default:
            break;
    }
}

int
ProgramTrustModeDrawer::rowCount() const
{
    return program_list_.size();
}

ImGuiTableColumnFlags
ProgramTrustModeDrawer::headerFlags(int column) const
{
    ImGuiTableColumnFlags flags = TableView::headerFlags(column);
    switch ((col_type)column)
    {
        case col_type::trust_mode:
            flags |= ImGuiTableColumnFlags_NoSort;
            break;
        case col_type::id:
            break;
        case col_type::update_date:
            flags |= ImGuiTableColumnFlags_DefaultSort;
            flags |= ImGuiTableColumnFlags_PreferSortDescending;
            break;
        default:
            flags |= ImGuiTableColumnFlags_WidthStretch;
            break;
    }
    return flags;
}

int
ProgramTrustModeDrawer::rowCompare(int left_row, int right_row, int column) const
{
    switch ((col_type)column)
    {
        case col_type::id:
            return program_list_[left_row].id - program_list_[right_row].id;
            break;
        case col_type::line:
            return program_list_[left_row].line.compare(program_list_[right_row].line);
            break;
        case col_type::station:
            return program_list_[left_row].station.compare(program_list_[right_row].station);
            break;
        case col_type::program:
            return program_list_[left_row].program.compare(program_list_[right_row].program);
            break;
        case col_type::create_date:
        {
            return (program_list_[left_row].create_date < program_list_[right_row].create_date)
                       ? -1
                       : ((program_list_[left_row].create_date > program_list_[right_row].create_date) ? 1 : 0);
        }
        break;
        case col_type::update_date:
            return (program_list_[left_row].update_date < program_list_[right_row].update_date)
                       ? -1
                       : ((program_list_[left_row].update_date > program_list_[right_row].update_date) ? 1 : 0);
            break;
        default:
            break;
    }
    return program_list_[left_row].id - program_list_[right_row].id;
}

bool
ProgramTrustModeDrawer::rowPassFilter(int row) const
{
    auto const& info = program_list_[row];
    switch (display_trust_type_)
    {
        case display_trust_type::all:
            break;
        case display_trust_type::trust:
            if (!info.trust)
            {
                return false;
            }
            break;
        case display_trust_type::no_trust:
            if (info.trust)
            {
                return false;
            }
            break;
        default:
            break;
    }

    return textFilter_.PassFilter(info.program.c_str());
}

bool
ProgramTrustModeDrawer::is_trust_unsafe(std::string_view line, std::string_view station, std::string_view program)
{

    refresh_db();
    if (!db_)
    {
        return false;
    }

    auto key = make_cache_key(line, station, program);
    auto iter = cache_index_.find(key);
    if (iter != cache_index_.end())
    {
        auto now = QDateTime::currentDateTime();

        auto& _info = program_list_[iter->second];
        _info.update_date = now;

        try
        {
            SQLite::Statement query(*db_,
                                    R"(UPDATE program SET update_date = (DATETIME('now', 'localtime')) WHERE id = ?)");
            query.bind(1, _info.id);
            query.exec();
        }
        catch (std::exception const& e)
        {
            common::Log::warn("更新检测时间异常: {}, line: {}, station: {}, program: {}",
                              common::misc::to_u8string(e.what()),
                              line,
                              station,
                              program);
        }
        return program_list_[iter->second].trust;
    }
    try
    {
        {
            SQLite::Statement query(*db_, R"(INSERT INTO program(line, station, program) VALUES (?,?,?))");
            query.bind(1, std::string(line));
            query.bind(2, std::string(station));
            query.bind(3, std::string(program));
            query.exec();
        }

        auto id = db_->getLastInsertRowid();

        SQLite::Statement query(*db_, R"(SELECT * FROM program WHERE id = ? LIMIT 1000)");
        query.bind(1, id);

        if (query.executeStep())
        {
            program_info item;
            item.id = query.getColumn("id").getInt64();
            item.line = query.getColumn("line").getString();
            item.station = query.getColumn("station").getString();
            item.program = query.getColumn("program").getString();
            item.trust = query.getColumn("trust").getInt();
            item.modified = false;
            item.create_date = QDateTime::fromString(QString::fromStdString(query.getColumn("create_date").getString()),
                                                     "yyyy-MM-dd hh:mm:ss");
            item.update_date = QDateTime::fromString(QString::fromStdString(query.getColumn("update_date").getString()),
                                                     "yyyy-MM-dd hh:mm:ss");

            auto index = program_list_.size();

            program_list_.push_back(item);
            cache_index_[key] = index;
            tableReloadRows();

            return item.trust;
        }
    }
    catch (std::exception const& e)
    {
        common::Log::warn("查询信任程序名异常: {}, line: {}, station: {}, program: {}",
                          common::misc::to_u8string(e.what()),
                          line,
                          station,
                          program);
    }
    return false;
}

void
ProgramTrustModeDrawer::draw(ImVec2 const& size)
{
    try
    {
        refresh_db();
    }
    catch (std::exception const& e)
    {
        common::Log::error("刷新信任程序列表异常:{}", common::misc::to_u8string(e.what()));
        return;
    }

    ImGui::PushID(this);

    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowCollapsed(true, ImGuiCond_FirstUseEver);
    if (ImGui::Begin(tr("Trust program configuration").toUtf8()))
    {
        ImGui::extend::BeginChild(
            [this]()
            {
                if (textFilter_.Draw(tr("Filter program name").toUtf8()))
                {
                    tableReloadRows();
                }
                if (ImGui::BeginCombo(tr("Filter").toUtf8(), display_trust_type_string(display_trust_type_).c_str()))
                {
                    if (ImGui::Selectable(tr("All").toUtf8(), display_trust_type_ == display_trust_type::all))
                    {
                        display_trust_type_ = display_trust_type::all;
                        tableReloadRows();
                    }
                    if (ImGui::Selectable(tr("Trust").toUtf8(), display_trust_type_ == display_trust_type::trust))
                    {
                        display_trust_type_ = display_trust_type::trust;
                        tableReloadRows();
                    }
                    if (ImGui::Selectable(tr("Distrust").toUtf8(), display_trust_type_ == display_trust_type::no_trust))
                    {
                        display_trust_type_ = display_trust_type::no_trust;
                        tableReloadRows();
                    }
                    ImGui::EndCombo();
                }

                // ImGui::SameLine();
                if (ImGui::Button(tr("Trust All").toUtf8()))
                {
                    set_trust_all(true);
                }
                ImGui::SameLine();
                if (ImGui::Button(tr("Distrust All").toUtf8()))
                {
                    set_trust_all(false);
                }

                auto const sz = ImGui::GetContentRegionAvail();
                ImVec2 outer_size_value = ImVec2(0, sz.y);
                TableView::draw(outer_size_value);
            },
            "program_trust_mode_drawer::draw",
            size,
            false);
    }
    ImGui::End();

    ImGui::PopID();
}

void
ProgramTrustModeDrawer::refresh_db()
{
    using namespace std::chrono_literals;
    // if(std::chrono::steady_clock::now() - last_update_time_ < 1s)
    //    return;
    // last_update_time_ = std::chrono::steady_clock::now();

    if (db_)
    {
        return;
    }
    try
    {
        db_ = detail::get_db();
        SQLite::Statement query(*db_, R"(SELECT * FROM program)");
        // query.bind(1, last_id);

        // bool has_new = false;
        while (query.executeStep())
        {
            program_info item;
            item.id = query.getColumn("id").getInt64();
            item.line = query.getColumn("line").getString();
            item.station = query.getColumn("station").getString();
            item.program = query.getColumn("program").getString();
            item.trust = query.getColumn("trust").getInt();
            item.modified = false;
            item.create_date = QDateTime::fromString(QString::fromStdString(query.getColumn("create_date").getString()),
                                                     "yyyy-MM-dd hh:mm:ss");
            item.update_date = QDateTime::fromString(QString::fromStdString(query.getColumn("update_date").getString()),
                                                     "yyyy-MM-dd hh:mm:ss");

            auto key = make_cache_key(item.line, item.station, item.program);
            auto index = program_list_.size();

            program_list_.push_back(item);
            cache_index_[key] = index;

            // has_new = true;
        }
        // if(has_new)
        //  table_reload_rows();
    }
    catch (std::exception const& e)
    {
        common::Log::warn("界面刷新信任的程序的列表异常:{}", common::misc::to_u8string(e.what()));
    }
}

void
ProgramTrustModeDrawer::apply_db()
{
    try
    {
        for (auto& item : program_list_)
        {
            if (!item.modified)
            {
                continue;
            }

            SQLite::Statement query(*db_, R"(UPDATE program SET trust = ? WHERE id = ?)");
            query.bind(1, item.trust ? 1 : 0);
            query.bind(2, item.id);
            query.exec();

            item.modified = false;
        }
    }
    catch (std::exception const& e)
    {
        common::Log::error(R"(Failed to update program trust: {})", common::misc::to_u8string(e.what()));
    }
}

void
ProgramTrustModeDrawer::set_trust_all(bool trust)
{
    auto const& rows = getDrawRowIndexes();

    for (auto const& row : rows)
    {
        auto& info = program_list_[row];

        if (info.trust == trust)
        {
            continue;
        }
        info.trust = trust;
        info.modified = true;
    }
    apply_db();
}

std::string
ProgramTrustModeDrawer::make_cache_key(std::string_view line, std::string_view station, std::string_view program)
{
    return std::format("{}_{}_{}", line, station, program);
}

void
ProgramTrustModeDrawer::clear()
{
    program_list_.clear();
    cache_index_.clear();
}

std::string
ProgramTrustModeDrawer::display_trust_type_string(ProgramTrustModeDrawer::display_trust_type& type)
{
    switch (type)
    {
        case ProgramTrustModeDrawer::display_trust_type::all:
            return tr("All").toStdString();
        case ProgramTrustModeDrawer::display_trust_type::trust:
            return tr("Trust").toStdString();
        case ProgramTrustModeDrawer::display_trust_type::no_trust:
            return tr("Distrust").toStdString();
        default:
            break;
    }
    return {};
}
