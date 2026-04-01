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

namespace detail
{

}

boost::json::value
DetectPanelImpl::toDetectJson() const
{
    boost::json::array json_board_info_array;
    boost::json::array json_component_info_array;

    for (auto const& board : m_boardItems)
    {
        boost::json::object json_board;
        json_board["index"] = board.m_boardIndex;
        json_board["sn"] = board.m_boardSn;
        json_board_info_array.push_back(std::move(json_board));

        for (auto const& comp : board.m_componentItems)
        {
            boost::json::array json_windows;
            for (auto const& win : comp.m_windowItems)
            {
                boost::json::object json_win;
                json_win["name"] = win.m_windowName;
                json_win["light"] = win.m_windowLight;
                json_win["defect"] = boost::json::array(win.m_defects.begin(), win.m_defects.end());
                json_win["image"] = win.m_base64Image;
                json_win["angle"] = win.m_angle;
                json_win["immediately_ng"] = win.m_immediatelyNg;

                boost::json::array json_detect_rect_array;
                for (auto const& rc : win.m_detectRects)
                {
                    json_detect_rect_array.push_back(boost::json::array { rc.tl().x, rc.tl().y, rc.br().x, rc.br().y });
                }
                json_win["detect_rect"] = json_detect_rect_array;

                boost::json::array json_train_rect_array;
                for (auto const& rc : win.m_trainRects)
                {
                    json_train_rect_array.push_back(boost::json::array { rc.tl().x, rc.tl().y, rc.br().x, rc.br().y });
                }
                json_win["train_rect"] = json_train_rect_array;

                json_windows.push_back(std::move(json_win));
            }

            boost::json::object json_comp;
            json_comp["name"] = comp.m_compName;
            json_comp["code"] = comp.m_compCode;
            json_comp["board_index"] = board.m_boardIndex;
            json_comp["window_info"] = json_windows;
            json_component_info_array.push_back(std::move(json_comp));
        }
    }

    boost::json::object json_doc;
    json_doc["line"] = m_line;
    json_doc["station"] = m_station;
    json_doc["name"] = m_name;
    json_doc["sn"] = m_sn;
    json_doc["board_info"] = json_board_info_array;
    json_doc["component_info"] = json_component_info_array;
    json_doc["enabled_trust_mode"] = this->m_enabledTrustMode;
    return json_doc;
}

void
DetectPanelImpl::setLine(std::string const& line)
{
    m_line = line;
}

void
DetectPanelImpl::setStation(std::string const& station)
{
    m_station = station;
}

void
DetectPanelImpl::setName(std::string const& name)
{
    m_name = name;
}

void
DetectPanelImpl::setSn(std::string const& sn)
{
    m_sn = sn;
}

void
DetectPanelImpl::setEnabledTrustMode(bool enabled)
{
    m_enabledTrustMode = enabled;
}

bool
DetectPanelImpl::isTrustModeEnabled() const
{
    return m_enabledTrustMode;
}

std::string const&
DetectPanelImpl::line() const
{
    return this->m_line;
}
std::string const&
DetectPanelImpl::station() const
{
    return this->m_station;
}
std::string const&
DetectPanelImpl::name() const
{
    return this->m_name;
}
std::string const&
DetectPanelImpl::sn() const
{
    return this->m_sn;
}
service::IAIAgentService::IDetectBoard*
DetectPanelImpl::mutableBoard(int board_index, std::string const& board_sn)
{
    if (auto _board = static_cast<DetectBoardImpl*>(findBoard(board_index)); _board)
    {
        if (!board_sn.empty())
        {
            _board->m_boardSn = board_sn;
        }
        return _board;
    }

    DetectBoardImpl _board;
    _board.m_boardIndex = board_index;
    _board.m_boardSn = board_sn;
    m_boardItems.push_back(std::move(_board));
    return &m_boardItems.back();
}

service::IAIAgentService::IDetectBoard*
DetectPanelImpl::findBoard(int board_index)
{
    auto iter = std::find_if(m_boardItems.begin(),
                             m_boardItems.end(),
                             [board_index](DetectBoardImpl const& board) { return board.m_boardIndex == board_index; });
    if (iter != m_boardItems.end())
    {
        return &(*iter);
    }

    return nullptr;
}

service::IAIAgentService::IDetectBoard const*
DetectPanelImpl::findBoard(int board_index) const
{
    return const_cast<DetectPanelImpl*>(this)->findBoard(board_index);
}

service::IAIAgentService::IDetectComponent const*
DetectPanelImpl::findComponent(int board_index, std::string const& comp_name) const
{
    return const_cast<DetectPanelImpl*>(this)->findComponent(board_index, comp_name);
}

service::IAIAgentService::IDetectComponent*
DetectPanelImpl::findComponent(int board_index, std::string const& comp_name)
{
    auto _board = findBoard(board_index);
    if (!_board)
    {
        return nullptr;
    }

    return _board->findComponent(comp_name);
}

service::IAIAgentService::IDetectWindow const*
DetectPanelImpl::findWindow(int board_index,
                            std::string const& comp_name,
                            std::string const& win_name,
                            std::string const& win_light) const
{
    return const_cast<DetectPanelImpl*>(this)->findWindow(board_index, comp_name, win_name, win_light);
}

service::IAIAgentService::IDetectWindow*
DetectPanelImpl::findWindow(int board_index,
                            std::string const& comp_name,
                            std::string const& win_name,
                            std::string const& win_light)
{
    auto _comp = findComponent(board_index, comp_name);
    if (!_comp)
    {
        return nullptr;
    }

    return _comp->findWindow(win_name, win_light);
}

std::vector<service::IAIAgentService::IDetectBoard const*>
DetectPanelImpl::boards() const
{
    std::vector<service::IAIAgentService::IDetectBoard const*> result;
    for (auto const& board : m_boardItems)
    {
        result.push_back(&board);
    }
    return result;
}

void
DetectPanelImpl::setComponentOpResult(int board_index, std::string const& comp_name, bool is_ok)
{
    auto _comp = static_cast<DetectComponentImpl*>(findComponent(board_index, comp_name));
    if (!_comp)
    {
        common::Log::warn(
            R"(找不到对应的元件设置OP结果: LINE: {} STATION: {} NAME: {} SN: {} BOARD_INDEX: {} COMP_NAME: {})",
            m_line,
            m_station,
            m_name,
            m_sn,
            board_index,
            comp_name);
        return;
    }
    _comp->m_isOpOk = is_ok;

    common::Log::info(
        R"(设置元件OP结果: LINE: {} STATION: {} NAME: {} SN: {} BOARD_INDEX: {} COMP_NAME: {} RESULT: {})",
        m_line,
        m_station,
        m_name,
        m_sn,
        board_index,
        comp_name,
        is_ok ? "P" : "F");
}

void
DetectPanelImpl::setWindowOpResult(int board_index,
                                   std::string const& comp_name,
                                   std::string const& win_name,
                                   std::string const& win_light,
                                   bool is_ok)
{
    auto _win = static_cast<DetectWindowImpl*>(findWindow(board_index, comp_name, win_name, win_light));
    if (!_win)
    {
        common::Log::warn(
            R"(找不到对应的窗口设置OP结果: LINE: {} STATION: {} NAME: {} SN: {} BOARD_INDEX: {} COMP_NAME: {} WIN_NAME: {} WIN_LIGHT: {})",
            m_line,
            m_station,
            m_name,
            m_sn,
            board_index,
            comp_name,
            win_name,
            win_light);
        return;
    }
    _win->m_isOpOk = is_ok;
    common::Log::info(
        R"(设置窗口OP结果: LINE: {} STATION: {} NAME: {} SN: {} BOARD_INDEX: {} COMP_NAME: {} WIN_NAME: {} WIN_LIGHT: {} RESULT: {})",
        m_line,
        m_station,
        m_name,
        m_sn,
        board_index,
        comp_name,
        win_name,
        win_light,
        is_ok ? "P" : "F");
}

boost::json::value
DetectPanelImpl::toOpResultJson() const
{
    boost::json::array json_component_info_array;
    for (auto const& board : m_boardItems)
    {
        for (auto const& comp : board.m_componentItems)
        {
            boost::json::array json_windows;
            for (auto const& win : comp.m_windowItems)
            {
                if (!win.m_isOpOk.has_value())
                {
                    continue;
                }

                boost::json::object json_win;
                json_win["name"] = win.m_windowName;
                json_win["light"] = win.m_windowLight;
                json_win["result"] = win.m_isOpOk.value() ? "P" : "F";
                json_windows.push_back(std::move(json_win));
            }
            if (!json_windows.empty())
            {
                boost::json::object json_comp;
                json_comp["name"] = comp.m_compName;
                json_comp["board_index"] = board.m_boardIndex;
                json_comp["window_info"] = json_windows;
                json_component_info_array.push_back(std::move(json_comp));
                continue;
            }
            if (!comp.m_isOpOk.has_value())
            {
                continue;
            }

            boost::json::object json_comp;
            json_comp["name"] = comp.m_compName;
            json_comp["board_index"] = board.m_boardIndex;
            json_comp["result"] = comp.m_isOpOk.value() ? "P" : "F";
            json_component_info_array.push_back(std::move(json_comp));
        }
    }
    boost::json::object json_doc;
    json_doc["line"] = m_line;
    json_doc["station"] = m_station;
    json_doc["name"] = m_name;
    json_doc["sn"] = m_sn;
    json_doc["component_info"] = json_component_info_array;
    return json_doc;
}

void
DetectPanelImpl::parseAiResult(boost::json::value const& json)
{
    for (auto const& json_comp : json.at("component_info").as_array())
    {
        int board_index = json_comp.at("board_index").to_number<int>();
        std::string comp_name(json_comp.at("name").as_string());
        std::string comp_result(json_comp.at("result").as_string());

        auto _comp = static_cast<DetectComponentImpl*>(findComponent(board_index, comp_name));
        if (!_comp)
        {
            common::Log::warn(
                R"(AI结果中找不到对应的元件: LINE: {} STATION: {} NAME: {} SN: {} BOARD_INDEX: {} COMP_NAME: {})",
                m_line,
                m_station,
                m_name,
                m_sn,
                board_index,
                comp_name);
            continue;
        }
        if (comp_result == "P")
        {
            common::Log::info(R"(AI结果元件OK: LINE: {} STATION: {} NAME: {} SN: {} BOARD_INDEX: {} COMP_NAME: {})",
                              m_line,
                              m_station,
                              m_name,
                              m_sn,
                              board_index,
                              comp_name);
            _comp->m_isAiOk = true;
        }

        for (auto const& json_win : json_comp.at("window_info").as_array())
        {
            std::string win_name(json_win.at("name").as_string());
            std::string win_light(json_win.at("light").as_string());
            std::string_view win_result = json_win.at("result").as_string();

            auto _win = static_cast<DetectWindowImpl*>(_comp->findWindow(win_name, win_light));
            if (!_win)
            {
                common::Log::warn(
                    R"(AI结果中找不到对应的窗口: LINE: {} STATION: {} NAME: {} SN: {} BOARD_INDEX: {} COMP_NAME: {} WIN_NAME: {} WIN_LIGHT: {})",
                    m_line,
                    m_station,
                    m_name,
                    m_sn,
                    board_index,
                    comp_name,
                    win_name,
                    win_light);
                continue;
            }
            if (win_result == "P")
            {
                common::Log::info(
                    R"(AI结果窗口OK: LINE: {} STATION: {} NAME: {} SN: {} BOARD_INDEX: {} COMP_NAME: {} WIN_NAME: {} WIN_LIGHT: {})",
                    m_line,
                    m_station,
                    m_name,
                    m_sn,
                    board_index,
                    comp_name,
                    win_name,
                    win_light);
                _win->m_isAiOk = true;
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

std::string const&
DetectBoardImpl::sn() const
{
    return m_boardSn;
}

int
DetectBoardImpl::index() const
{
    return m_boardIndex;
}

service::IAIAgentService::IDetectComponent const*
DetectBoardImpl::findComponent(std::string const& comp_name) const
{
    return const_cast<DetectBoardImpl*>(this)->findComponent(comp_name);
}

service::IAIAgentService::IDetectComponent*
DetectBoardImpl::findComponent(std::string const& comp_name)
{
    auto iter = std::find_if(m_componentItems.begin(),
                             m_componentItems.end(),
                             [&](DetectComponentImpl const& comp) { return comp.m_compName == comp_name; });
    if (iter != m_componentItems.end())
    {
        return &(*iter);
    }

    return nullptr;
}

service::IAIAgentService::IDetectWindow const*
DetectBoardImpl::findWindow(std::string const& comp_name,
                            std::string const& win_name,
                            std::string const& win_light) const
{
    auto _comp = findComponent(comp_name);
    if (!_comp)
    {
        return nullptr;
    }

    return _comp->findWindow(win_name, win_light);
}

service::IAIAgentService::IDetectComponent*
DetectBoardImpl::mutableComponent(std::string const& comp_name, std::string const& comp_code)
{
    if (auto _comp = static_cast<DetectComponentImpl*>(findComponent(comp_name)); _comp)
    {
        _comp->m_compCode = comp_code;
        return _comp;
    }

    DetectComponentImpl _comp;
    _comp.m_compName = comp_name;
    _comp.m_compCode = comp_code;
    m_componentItems.push_back(std::move(_comp));
    return &m_componentItems.back();
}

std::vector<service::IAIAgentService::IDetectComponent const*>
DetectBoardImpl::components() const
{
    std::vector<service::IAIAgentService::IDetectComponent const*> result;
    for (auto const& comp : m_componentItems)
    {
        result.push_back(&comp);
    }
    return result;
}

bool
DetectComponentImpl::isAiOk() const
{
    return m_isAiOk;
}

service::IAIAgentService::IDetectWindow const*
DetectComponentImpl::findWindow(std::string const& win_name, std::string const& win_light) const
{
    return const_cast<DetectComponentImpl*>(this)->findWindow(win_name, win_light);
}

service::IAIAgentService::IDetectWindow*
DetectComponentImpl::findWindow(std::string const& win_name, std::string const& win_light)
{
    auto iter = std::find_if(m_windowItems.begin(),
                             m_windowItems.end(),
                             [&](DetectWindowImpl const& win)
                             { return win.m_windowName == win_name && win.m_windowLight == win_light; });
    if (iter != m_windowItems.end())
    {
        return &(*iter);
    }
    return nullptr;
}

service::IAIAgentService::IDetectWindow*
DetectComponentImpl::mutableWindow(std::string const& win_name, std::string const& win_light)
{
    if (auto _window = findWindow(win_name, win_light); _window)
    {
        return _window;
    }
    DetectWindowImpl _window;
    _window.m_windowName = win_name;
    _window.m_windowLight = win_light;
    m_windowItems.push_back(std::move(_window));
    return &m_windowItems.back();
}

std::string const&
DetectComponentImpl::name() const
{
    return m_compName;
}

std::string const&
DetectComponentImpl::code() const
{
    return m_compCode;
}
std::vector<service::IAIAgentService::IDetectWindow const*>
DetectComponentImpl::windows() const
{
    std::vector<service::IAIAgentService::IDetectWindow const*> result;
    for (auto const& win : m_windowItems)
    {
        result.push_back(&win);
    }
    return result;
}

std::string const&
DetectWindowImpl::name() const
{
    return this->m_windowName;
}
std::string const&
DetectWindowImpl::light() const
{
    return this->m_windowLight;
}
bool
DetectWindowImpl::isImmediatelyNg() const
{
    return m_immediatelyNg;
}

bool
DetectWindowImpl::isAiOk() const
{
    return m_isAiOk;
}

void
DetectWindowImpl::setImage(cv::Mat const& image)
{
    throw std::logic_error("The method or operation is not implemented.");
}

void
DetectWindowImpl::setBase64Image(std::string&& base64_image)
{
    this->m_base64Image = std::move(base64_image);
}

void
DetectWindowImpl::setFileImage(std::string const& file_path)
{
    throw std::logic_error("The method or operation is not implemented.");
}

void
DetectWindowImpl::setDefects(std::set<std::string> const& defects)
{
    this->m_defects = defects;
}

void
DetectWindowImpl::setDetectRois(std::vector<cv::Rect> const& rois)
{
    this->m_detectRects.assign(rois.begin(), rois.end());
}

void
DetectWindowImpl::setTrainRois(std::vector<cv::Rect> const& rois)
{
    this->m_trainRects.assign(rois.begin(), rois.end());
}

void
DetectWindowImpl::setAngle(int angle)
{
    this->m_angle = angle;
}

void
DetectWindowImpl::setImmediatelyNg(bool immediately_ng)
{
    this->m_immediatelyNg = immediately_ng;
}