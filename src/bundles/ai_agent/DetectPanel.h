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
    std::string window_name;
    std::string window_light;
    std::set<std::string> defects;
    std::string base64_image;
    std::list<cv::Rect> detect_rects;
    std::list<cv::Rect> train_rects;
    int angle           = 0;
    bool immediately_ng = false;

    bool is_ai_ok = false;
    std::optional<bool> is_op_ok;

    const std::string& name() const override;
    const std::string& light() const override;

    bool isImmediatelyNg() const override;

    bool isAiOk() const override;

    void setImage(const cv::Mat& image) override;

    void setBase64Image(std::string&& base64_image) override;

    void setFileImage(const std::string& file_path) override;

    void setDefects(const std::set<std::string>& defects) override;

    void setDetectRois(const std::vector<cv::Rect>& rois) override;

    void setTrainRois(const std::vector<cv::Rect>& rois) override;

    void setAngle(int angle) override;

    void setImmediatelyNg(bool immediately_ng) override;
};

struct DetectComponentImpl : public service::IAIAgentService::IDetectComponent
{
    std::string comp_name;
    std::string comp_code;
    std::list<DetectWindowImpl> window_items;

    bool is_ai_ok = false;
    std::optional<bool> is_op_ok;

    bool isAiOk() const override;

    const service::IAIAgentService::IDetectWindow*
    findWindow(const std::string& win_name, const std::string& win_light) const override;
    service::IAIAgentService::IDetectWindow* findWindow(const std::string& win_name,
                                                        const std::string& win_light) override;

    service::IAIAgentService::IDetectWindow* mutableWindow(const std::string& win_name,
                                                           const std::string& win_light) override;

    const std::string& name() const override;

    const std::string& code() const override;

    std::vector<const service::IAIAgentService::IDetectWindow*> windows() const override;
};
struct DetectBoardImpl : public service::IAIAgentService::IDetectBoard
{
public:
    const std::string& sn() const override;

    int index() const override;

    const service::IAIAgentService::IDetectComponent*
    findComponent(const std::string& comp_name) const override;
    service::IAIAgentService::IDetectComponent*
    findComponent(const std::string& comp_name) override;

    const service::IAIAgentService::IDetectWindow*
    findWindow(const std::string& comp_name,
               const std::string& win_name,
               const std::string& win_light) const override;

    service::IAIAgentService::IDetectComponent*
    mutableComponent(const std::string& comp_name, const std::string& comp_code) override;

    std::vector<const service::IAIAgentService::IDetectComponent*> components() const override;


    int board_index = -1;
    std::string board_sn;
    std::list<DetectComponentImpl> component_items;
};
struct DetectPanelImpl : public service::IAIAgentService::IDetectPanel
{
    const std::string& line() const override;
    const std::string& station() const override;
    const std::string& name() const override;
    const std::string& sn() const override;

    service::IAIAgentService::IDetectBoard* mutableBoard(int board_index,
                                                         const std::string& board_sn) override;

    service::IAIAgentService::IDetectBoard* findBoard(int board_index) override;

    void setComponentOpResult(int board_index, const std::string& comp_name, bool is_ok) override;

    void setWindowOpResult(int board_index,
                           const std::string& comp_name,
                           const std::string& win_name,
                           const std::string& win_light,
                           bool is_ok) override;

    boost::json::value toOpResultJson() const;
    boost::json::value toDetectJson() const;

    void parseAiResult(const boost::json::value& json);

    // boost::asio::awaitable<void>
    // proc_detect(std::string_view ip,
    //             int port,
    //             const std::optional<std::chrono::seconds>& timeout_seconds = std::nullopt);

    // boost::asio::awaitable<void> proc_op_result(std::string_view ip, int port) const;

    std::string line_;
    std::string station_;
    std::string name_;
    std::string sn_;
    bool enabled_trust_mode_ = true;
    std::list<DetectBoardImpl> board_items;

    void setLine(const std::string& line) override;

    void setStation(const std::string& station) override;

    void setName(const std::string& name) override;

    void setSn(const std::string& sn) override;

    void setEnabledTrustMode(bool enabled) override;

    bool isTrustModeEnabled() const override;

    const service::IAIAgentService::IDetectBoard* findBoard(int board_index) const override;

    const service::IAIAgentService::IDetectComponent*
    findComponent(int board_index, const std::string& comp_name) const override;

    const service::IAIAgentService::IDetectWindow*
    findWindow(int board_index,
               const std::string& comp_name,
               const std::string& win_name,
               const std::string& win_light) const override;

    std::vector<const service::IAIAgentService::IDetectBoard*> boards() const override;

    service::IAIAgentService::IDetectComponent*
    findComponent(int board_index, const std::string& comp_name) override;

    service::IAIAgentService::IDetectWindow* findWindow(int board_index,
                                                        const std::string& comp_name,
                                                        const std::string& win_name,
                                                        const std::string& win_light) override;
};