#include "DetectPanel.h"
#include "common/Logger.h"

// namespace detail {
//
//
// static auto get_client(std::string_view ip,
//                        int port,
//                        std::string_view path,
//                        const std::optional<std::chrono::seconds>& timeout_seconds)
//{
//     spdlog::debug("正在发送信息给AI: http://{}:{}{}", ip, port, path);
//     auto pool = Application::instance().httpClientPool();
//     auto cli  = pool->acquire(ip, port);
//     if (timeout_seconds) {
//         cli->set_timeout(*timeout_seconds);
//         cli->set_timeout_policy(httplib::client::http_client::timeout_policy::overall);
//     }
//     else
//         cli->set_timeout_policy(httplib::client::http_client::timeout_policy::never);
//
//     return cli;
// }
// static std::optional<boost::json::value>
// proc_json_response(const httplib::client::multi_http_client_pool::ClientHandle& cli,
//                    std::string_view path,
//                    const httplib::client::http_client::response_result& response)
//{
//     if (!response) {
//         spdlog::warn("给AI发送请求失败: http://{}:{}{} 错误: {}",
//                      cli->host(),
//                      cli->port(),
//                      path,
//                      misc::to_u8string(response.error().message()));
//         return std::nullopt;
//     }
//
//     if (response->result() != httplib::http::status::ok) {
//         spdlog::warn("收到的AI回复出现错误: http://{}:{}{} 错误: {}",
//                      cli->host(),
//                      cli->port(),
//                      path,
//                      misc::to_u8string(httplib::http::obsolete_reason(response->result())));
//         return std::nullopt;
//     }
//     spdlog::debug("发送信息给AI成功:  http://{}:{}{}", cli->host(), cli->port(), path);
//     return response->body().as<httplib::body::json_body>();
// }
// boost::asio::awaitable<std::optional<boost::json::value>>
// send_ai_post_request(std::string_view ip,
//                      int port,
//                      std::string_view path,
//                      boost::json::value&& doc,
//                      const std::optional<std::chrono::seconds>& timeout_seconds = std::nullopt)
//{
//     auto cli      = get_client(ip, port, path, timeout_seconds);
//     auto response = co_await cli->async_post(path, std::move(doc));
//     co_return proc_json_response(cli, path, response);
// }
// } // namespace detail

namespace detail {

}

boost::json::value DetectPanelImpl::toDetectJson() const
{
    boost::json::array json_board_info_array;
    boost::json::array json_component_info_array;

    for (const auto& board : board_items) {
        boost::json::object json_board;
        json_board["index"] = board.board_index;
        json_board["sn"]    = board.board_sn;
        json_board_info_array.push_back(std::move(json_board));

        for (const auto& comp : board.component_items) {
            boost::json::array json_windows;
            for (const auto& win : comp.window_items) {
                boost::json::object json_win;
                json_win["name"]   = win.window_name;
                json_win["light"]  = win.window_light;
                json_win["defect"] = boost::json::array(win.defects.begin(), win.defects.end());
                json_win["image"]  = win.base64_image;
                json_win["angle"]  = win.angle;
                json_win["immediately_ng"] = win.immediately_ng;

                boost::json::array json_detect_rect_array;
                for (const auto& rc : win.detect_rects)
                    json_detect_rect_array.push_back(
                        boost::json::array {rc.tl().x, rc.tl().y, rc.br().x, rc.br().y});
                json_win["detect_rect"] = json_detect_rect_array;

                boost::json::array json_train_rect_array;
                for (const auto& rc : win.train_rects)
                    json_train_rect_array.push_back(
                        boost::json::array {rc.tl().x, rc.tl().y, rc.br().x, rc.br().y});
                json_win["train_rect"] = json_train_rect_array;

                json_windows.push_back(std::move(json_win));
            }

            boost::json::object json_comp;
            json_comp["name"]        = comp.comp_name;
            json_comp["code"]        = comp.comp_code;
            json_comp["board_index"] = board.board_index;
            json_comp["window_info"] = json_windows;
            json_component_info_array.push_back(std::move(json_comp));
        }
    }

    boost::json::object json_doc;
    json_doc["line"]               = line_;
    json_doc["station"]            = station_;
    json_doc["name"]               = name_;
    json_doc["sn"]                 = sn_;
    json_doc["board_info"]         = json_board_info_array;
    json_doc["component_info"]     = json_component_info_array;
    json_doc["enabled_trust_mode"] = this->enabled_trust_mode_;
    return json_doc;
}

void DetectPanelImpl::setLine(const std::string& line)
{
    line_ = line;
}

void DetectPanelImpl::setStation(const std::string& station)
{
    station_ = station;
}

void DetectPanelImpl::setName(const std::string& name)
{
    name_ = name;
}

void DetectPanelImpl::setSn(const std::string& sn)
{
    sn_ = sn;
}

void DetectPanelImpl::setEnabledTrustMode(bool enabled)
{
    enabled_trust_mode_ = enabled;
}

bool DetectPanelImpl::isTrustModeEnabled() const
{
    return enabled_trust_mode_;
}

const std::string& DetectPanelImpl::line() const
{
    return this->line_;
}
const std::string& DetectPanelImpl::station() const
{
    return this->station_;
}
const std::string& DetectPanelImpl::name() const
{
    return this->name_;
}
const std::string& DetectPanelImpl::sn() const
{
    return this->sn_;
}
service::IAIAgentService::IDetectBoard* DetectPanelImpl::mutableBoard(int board_index,
                                                                      const std::string& board_sn)
{
    if (auto _board = static_cast<DetectBoardImpl*>(findBoard(board_index)); _board) {
        if (!board_sn.empty())
            _board->board_sn = board_sn;
        return _board;
    }

    DetectBoardImpl _board;
    _board.board_index = board_index;
    _board.board_sn    = board_sn;
    board_items.push_back(std::move(_board));
    return &board_items.back();
}

service::IAIAgentService::IDetectBoard* DetectPanelImpl::findBoard(int board_index)
{
    auto iter = std::find_if(
        board_items.begin(), board_items.end(), [board_index](const DetectBoardImpl& board) {
            return board.board_index == board_index;
        });
    if (iter != board_items.end())
        return &(*iter);

    return nullptr;
}

const service::IAIAgentService::IDetectBoard* DetectPanelImpl::findBoard(int board_index) const
{
    return const_cast<DetectPanelImpl*>(this)->findBoard(board_index);
}

const service::IAIAgentService::IDetectComponent*
DetectPanelImpl::findComponent(int board_index, const std::string& comp_name) const
{
    return const_cast<DetectPanelImpl*>(this)->findComponent(board_index, comp_name);
}

service::IAIAgentService::IDetectComponent*
DetectPanelImpl::findComponent(int board_index, const std::string& comp_name)
{
    auto _board = findBoard(board_index);
    if (!_board)
        return nullptr;

    return _board->findComponent(comp_name);
}

const service::IAIAgentService::IDetectWindow*
DetectPanelImpl::findWindow(int board_index,
                            const std::string& comp_name,
                            const std::string& win_name,
                            const std::string& win_light) const
{
    return const_cast<DetectPanelImpl*>(this)->findWindow(
        board_index, comp_name, win_name, win_light);
}

service::IAIAgentService::IDetectWindow* DetectPanelImpl::findWindow(int board_index,
                                                                     const std::string& comp_name,
                                                                     const std::string& win_name,
                                                                     const std::string& win_light)
{
    auto _comp = findComponent(board_index, comp_name);
    if (!_comp)
        return nullptr;

    return _comp->findWindow(win_name, win_light);
}

std::vector<const service::IAIAgentService::IDetectBoard*> DetectPanelImpl::boards() const
{
    std::vector<const service::IAIAgentService::IDetectBoard*> result;
    for (const auto& board : board_items)
        result.push_back(&board);
    return result;
}

void DetectPanelImpl::setComponentOpResult(int board_index,
                                           const std::string& comp_name,
                                           bool is_ok)
{
    auto _comp = static_cast<DetectComponentImpl*>(findComponent(board_index, comp_name));
    if (!_comp) {
        common::logger::warn(
            R"(找不到对应的元件设置OP结果: LINE: {} STATION: {} NAME: {} SN: {} BOARD_INDEX: {} COMP_NAME: {})",
            line_,
            station_,
            name_,
            sn_,
            board_index,
            comp_name);
        return;
    }
    _comp->is_op_ok = is_ok;

    common::logger::info(
        R"(设置元件OP结果: LINE: {} STATION: {} NAME: {} SN: {} BOARD_INDEX: {} COMP_NAME: {} RESULT: {})",
        line_,
        station_,
        name_,
        sn_,
        board_index,
        comp_name,
        is_ok ? "P" : "F");
}

void DetectPanelImpl::setWindowOpResult(int board_index,
                                        const std::string& comp_name,
                                        const std::string& win_name,
                                        const std::string& win_light,
                                        bool is_ok)
{
    auto _win =
        static_cast<DetectWindowImpl*>(findWindow(board_index, comp_name, win_name, win_light));
    if (!_win) {
        common::logger::warn(
            R"(找不到对应的窗口设置OP结果: LINE: {} STATION: {} NAME: {} SN: {} BOARD_INDEX: {} COMP_NAME: {} WIN_NAME: {} WIN_LIGHT: {})",
            line_,
            station_,
            name_,
            sn_,
            board_index,
            comp_name,
            win_name,
            win_light);
        return;
    }
    _win->is_op_ok = is_ok;
    common::logger::info(
        R"(设置窗口OP结果: LINE: {} STATION: {} NAME: {} SN: {} BOARD_INDEX: {} COMP_NAME: {} WIN_NAME: {} WIN_LIGHT: {} RESULT: {})",
        line_,
        station_,
        name_,
        sn_,
        board_index,
        comp_name,
        win_name,
        win_light,
        is_ok ? "P" : "F");
}

boost::json::value DetectPanelImpl::toOpResultJson() const
{
    boost::json::array json_component_info_array;
    for (const auto& board : board_items) {
        for (const auto& comp : board.component_items) {
            boost::json::array json_windows;
            for (const auto& win : comp.window_items) {
                if (!win.is_op_ok.has_value())
                    continue;

                boost::json::object json_win;
                json_win["name"]   = win.window_name;
                json_win["light"]  = win.window_light;
                json_win["result"] = win.is_op_ok.value() ? "P" : "F";
                json_windows.push_back(std::move(json_win));
            }
            if (!json_windows.empty()) {
                boost::json::object json_comp;
                json_comp["name"]        = comp.comp_name;
                json_comp["board_index"] = board.board_index;
                json_comp["window_info"] = json_windows;
                json_component_info_array.push_back(std::move(json_comp));
                continue;
            }
            if (!comp.is_op_ok.has_value())
                continue;

            boost::json::object json_comp;
            json_comp["name"]        = comp.comp_name;
            json_comp["board_index"] = board.board_index;
            json_comp["result"]      = comp.is_op_ok.value() ? "P" : "F";
            json_component_info_array.push_back(std::move(json_comp));
        }
    }
    boost::json::object json_doc;
    json_doc["line"]           = line_;
    json_doc["station"]        = station_;
    json_doc["name"]           = name_;
    json_doc["sn"]             = sn_;
    json_doc["component_info"] = json_component_info_array;
    return json_doc;
}

void DetectPanelImpl::parseAiResult(const boost::json::value& json)
{
    for (const auto& json_comp : json.at("component_info").as_array()) {
        int board_index = json_comp.at("board_index").to_number<int>();
        std::string comp_name(json_comp.at("name").as_string());
        std::string comp_result(json_comp.at("result").as_string());

        auto _comp = static_cast<DetectComponentImpl*>(findComponent(board_index, comp_name));
        if (!_comp) {
            common::logger::warn(
                R"(AI结果中找不到对应的元件: LINE: {} STATION: {} NAME: {} SN: {} BOARD_INDEX: {} COMP_NAME: {})",
                line_,
                station_,
                name_,
                sn_,
                board_index,
                comp_name);
            continue;
        }
        if (comp_result == "P") {
            common::logger::info(
                R"(AI结果元件OK: LINE: {} STATION: {} NAME: {} SN: {} BOARD_INDEX: {} COMP_NAME: {})",
                line_,
                station_,
                name_,
                sn_,
                board_index,
                comp_name);
            _comp->is_ai_ok = true;
        }

        for (const auto& json_win : json_comp.at("window_info").as_array()) {
            std::string win_name(json_win.at("name").as_string());
            std::string win_light(json_win.at("light").as_string());
            std::string_view win_result = json_win.at("result").as_string();

            auto _win = static_cast<DetectWindowImpl*>(_comp->findWindow(win_name, win_light));
            if (!_win) {
                common::logger::warn(
                    R"(AI结果中找不到对应的窗口: LINE: {} STATION: {} NAME: {} SN: {} BOARD_INDEX: {} COMP_NAME: {} WIN_NAME: {} WIN_LIGHT: {})",
                    line_,
                    station_,
                    name_,
                    sn_,
                    board_index,
                    comp_name,
                    win_name,
                    win_light);
                continue;
            }
            if (win_result == "P") {
                common::logger::info(
                    R"(AI结果窗口OK: LINE: {} STATION: {} NAME: {} SN: {} BOARD_INDEX: {} COMP_NAME: {} WIN_NAME: {} WIN_LIGHT: {})",
                    line_,
                    station_,
                    name_,
                    sn_,
                    board_index,
                    comp_name,
                    win_name,
                    win_light);
                _win->is_ai_ok = true;
            }
        }
    }
}

// boost::asio::awaitable<void> detect_panel::proc_detect(
//     std::string_view ip,
//     int port,
//     const std::optional<std::chrono::seconds>& timeout_seconds /*= std::nullopt*/)
//{
//     auto response = co_await detail::send_ai_post_request(
//         ip, port, "/collect/sync", to_detect_json(), timeout_seconds);
//
//     if (enabled_trust_mode && response) {
//         parse_ai_result(*response);
//     }
// }
//
// boost::asio::awaitable<void> detect_panel::proc_op_result(std::string_view ip, int port) const
//{
//     spdlog::info("给AI发送复判 LINE: {} STATION: {} NAME: {} SN: {}", line, station, name, sn);
//     co_await detail::send_ai_post_request(ip, port, "/op_result", to_op_result_json());
// }

const std::string& DetectBoardImpl::sn() const
{
    return board_sn;
}

int DetectBoardImpl::index() const
{
    return board_index;
}

const service::IAIAgentService::IDetectComponent*
DetectBoardImpl::findComponent(const std::string& comp_name) const
{
    return const_cast<DetectBoardImpl*>(this)->findComponent(comp_name);
}

service::IAIAgentService::IDetectComponent*
DetectBoardImpl::findComponent(const std::string& comp_name)
{
    auto iter =
        std::find_if(component_items.begin(),
                     component_items.end(),
                     [&](const DetectComponentImpl& comp) { return comp.comp_name == comp_name; });
    if (iter != component_items.end())
        return &(*iter);

    return nullptr;
}

const service::IAIAgentService::IDetectWindow* DetectBoardImpl::findWindow(
    const std::string& comp_name, const std::string& win_name, const std::string& win_light) const
{
    auto _comp = findComponent(comp_name);
    if (!_comp)
        return nullptr;

    return _comp->findWindow(win_name, win_light);
}

service::IAIAgentService::IDetectComponent*
DetectBoardImpl::mutableComponent(const std::string& comp_name, const std::string& comp_code)
{
    if (auto _comp = static_cast<DetectComponentImpl*>(findComponent(comp_name)); _comp) {
        _comp->comp_code = comp_code;
        return _comp;
    }

    DetectComponentImpl _comp;
    _comp.comp_name = comp_name;
    _comp.comp_code = comp_code;
    component_items.push_back(std::move(_comp));
    return &component_items.back();
}

std::vector<const service::IAIAgentService::IDetectComponent*> DetectBoardImpl::components() const
{
    std::vector<const service::IAIAgentService::IDetectComponent*> result;
    for (const auto& comp : component_items)
        result.push_back(&comp);
    return result;
}

bool DetectComponentImpl::isAiOk() const
{
    return is_ai_ok;
}

const service::IAIAgentService::IDetectWindow*
DetectComponentImpl::findWindow(const std::string& win_name, const std::string& win_light) const
{
    return const_cast<DetectComponentImpl*>(this)->findWindow(win_name, win_light);
}

service::IAIAgentService::IDetectWindow*
DetectComponentImpl::findWindow(const std::string& win_name, const std::string& win_light)
{
    auto iter =
        std::find_if(window_items.begin(), window_items.end(), [&](const DetectWindowImpl& win) {
            return win.window_name == win_name && win.window_light == win_light;
        });
    if (iter != window_items.end())
        return &(*iter);
    return nullptr;
}

service::IAIAgentService::IDetectWindow*
DetectComponentImpl::mutableWindow(const std::string& win_name, const std::string& win_light)
{
    if (auto _window = findWindow(win_name, win_light); _window)
        return _window;
    DetectWindowImpl _window;
    _window.window_name  = win_name;
    _window.window_light = win_light;
    window_items.push_back(std::move(_window));
    return &window_items.back();
}

const std::string& DetectComponentImpl::name() const
{
    return comp_name;
}

const std::string& DetectComponentImpl::code() const
{
    return comp_code;
}
std::vector<const service::IAIAgentService::IDetectWindow*> DetectComponentImpl::windows() const
{
    std::vector<const service::IAIAgentService::IDetectWindow*> result;
    for (const auto& win : window_items)
        result.push_back(&win);
    return result;
}

const std::string& DetectWindowImpl::name() const
{
    return this->window_name;
}
const std::string& DetectWindowImpl::light() const
{
    return this->window_light;
}
bool DetectWindowImpl::isImmediatelyNg() const
{
    return immediately_ng;
}

bool DetectWindowImpl::isAiOk() const
{
    return is_ai_ok;
}

void DetectWindowImpl::setImage(const cv::Mat& image)
{
    throw std::logic_error("The method or operation is not implemented.");
}

void DetectWindowImpl::setBase64Image(std::string&& base64_image)
{
    this->base64_image = std::move(base64_image);
}

void DetectWindowImpl::setFileImage(const std::string& file_path)
{
    throw std::logic_error("The method or operation is not implemented.");
}

void DetectWindowImpl::setDefects(const std::set<std::string>& defects)
{
    this->defects = defects;
}

void DetectWindowImpl::setDetectRois(const std::vector<cv::Rect>& rois)
{
    this->detect_rects.assign(rois.begin(), rois.end());
}

void DetectWindowImpl::setTrainRois(const std::vector<cv::Rect>& rois)
{
    this->train_rects.assign(rois.begin(), rois.end());
}

void DetectWindowImpl::setAngle(int angle)
{
    this->angle = angle;
}

void DetectWindowImpl::setImmediatelyNg(bool immediately_ng)
{
    this->immediately_ng = immediately_ng;
}