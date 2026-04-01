#pragma once
#include "service/IAIAgentService.h"
#include <boost/asio/awaitable.hpp>
#include <boost/json/value.hpp>
#include <list>
#include <opencv2/opencv.hpp>
#include <optional>
#include <set>
#include <string>
#include <string_view>

struct DetectWindowImpl : public service::IAIAgentService::IDetectWindow
{
    std::string m_windowName;
    std::string m_windowLight;
    std::set<std::string> m_defects;
    std::string m_base64Image;
    std::list<cv::Rect> m_detectRects;
    std::list<cv::Rect> m_trainRects;
    int m_angle = 0;
    bool m_immediatelyNg = false;

    bool m_isAiOk = false;
    std::optional<bool> m_isOpOk;

    std::string const& name() const override;
    std::string const& light() const override;

    bool isImmediatelyNg() const override;

    bool isAiOk() const override;

    void setImage(cv::Mat const& image) override;

    void setBase64Image(std::string&& base64_image) override;

    void setFileImage(std::string const& file_path) override;

    void setDefects(std::set<std::string> const& defects) override;

    void setDetectRois(std::vector<cv::Rect> const& rois) override;

    void setTrainRois(std::vector<cv::Rect> const& rois) override;

    void setAngle(int angle) override;

    void setImmediatelyNg(bool immediately_ng) override;
};

struct DetectComponentImpl : public service::IAIAgentService::IDetectComponent
{
    std::string m_compName;
    std::string m_compCode;
    std::list<DetectWindowImpl> m_windowItems;

    bool m_isAiOk = false;
    std::optional<bool> m_isOpOk;

    bool isAiOk() const override;

    service::IAIAgentService::IDetectWindow const* findWindow(std::string const& win_name,
                                                              std::string const& win_light) const override;
    service::IAIAgentService::IDetectWindow* findWindow(std::string const& win_name,
                                                        std::string const& win_light) override;

    service::IAIAgentService::IDetectWindow* mutableWindow(std::string const& win_name,
                                                           std::string const& win_light) override;

    std::string const& name() const override;

    std::string const& code() const override;

    std::vector<service::IAIAgentService::IDetectWindow const*> windows() const override;
};
struct DetectBoardImpl : public service::IAIAgentService::IDetectBoard
{
  public:
    std::string const& sn() const override;

    int index() const override;

    service::IAIAgentService::IDetectComponent const* findComponent(std::string const& comp_name) const override;
    service::IAIAgentService::IDetectComponent* findComponent(std::string const& comp_name) override;

    service::IAIAgentService::IDetectWindow const* findWindow(std::string const& comp_name,
                                                              std::string const& win_name,
                                                              std::string const& win_light) const override;

    service::IAIAgentService::IDetectComponent* mutableComponent(std::string const& comp_name,
                                                                 std::string const& comp_code) override;

    std::vector<service::IAIAgentService::IDetectComponent const*> components() const override;

    int m_boardIndex = -1;
    std::string m_boardSn;
    std::list<DetectComponentImpl> m_componentItems;
};
struct DetectPanelImpl : public service::IAIAgentService::IDetectPanel
{
    std::string const& line() const override;
    std::string const& station() const override;
    std::string const& name() const override;
    std::string const& sn() const override;

    service::IAIAgentService::IDetectBoard* mutableBoard(int board_index, std::string const& board_sn) override;

    service::IAIAgentService::IDetectBoard* findBoard(int board_index) override;

    void setComponentOpResult(int board_index, std::string const& comp_name, bool is_ok) override;

    void setWindowOpResult(int board_index,
                           std::string const& comp_name,
                           std::string const& win_name,
                           std::string const& win_light,
                           bool is_ok) override;

    boost::json::value toOpResultJson() const;
    boost::json::value toDetectJson() const;

    void parseAiResult(boost::json::value const& json);

    // boost::asio::awaitable<void>
    // proc_detect(std::string_view ip,
    //             int port,
    //             const std::optional<std::chrono::seconds>& timeout_seconds = std::nullopt);

    // boost::asio::awaitable<void> proc_op_result(std::string_view ip, int port) const;

    std::string m_line;
    std::string m_station;
    std::string m_name;
    std::string m_sn;
    bool m_enabledTrustMode = true;
    std::list<DetectBoardImpl> m_boardItems;

    void setLine(std::string const& line) override;

    void setStation(std::string const& station) override;

    void setName(std::string const& name) override;

    void setSn(std::string const& sn) override;

    void setEnabledTrustMode(bool enabled) override;

    bool isTrustModeEnabled() const override;

    service::IAIAgentService::IDetectBoard const* findBoard(int board_index) const override;

    service::IAIAgentService::IDetectComponent const* findComponent(int board_index,
                                                                    std::string const& comp_name) const override;

    service::IAIAgentService::IDetectWindow const* findWindow(int board_index,
                                                              std::string const& comp_name,
                                                              std::string const& win_name,
                                                              std::string const& win_light) const override;

    std::vector<service::IAIAgentService::IDetectBoard const*> boards() const override;

    service::IAIAgentService::IDetectComponent* findComponent(int board_index, std::string const& comp_name) override;

    service::IAIAgentService::IDetectWindow* findWindow(int board_index,
                                                        std::string const& comp_name,
                                                        std::string const& win_name,
                                                        std::string const& win_light) override;
};