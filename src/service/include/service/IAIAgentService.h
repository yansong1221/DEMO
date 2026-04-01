#pragma once
#include <opencv2/opencv.hpp>
#include <set>
#include <vector>

namespace service {

class IAIAgentService
{
public:
    class IDetectWindow
    {
    public:
        virtual ~IDetectWindow()                 = default;
        virtual const std::string& name() const  = 0;
        virtual const std::string& light() const = 0;
        virtual bool isImmediatelyNg() const     = 0;
        virtual bool isAiOk() const              = 0;

        virtual void setImage(const cv::Mat& image)             = 0;
        virtual void setBase64Image(std::string&& base64_image) = 0;
        virtual void setFileImage(const std::string& file_path) = 0;

        virtual void setDefects(const std::set<std::string>& defects) = 0;
        virtual void setDetectRois(const std::vector<cv::Rect>& rois) = 0;
        virtual void setTrainRois(const std::vector<cv::Rect>& rois)  = 0;
        virtual void setAngle(int angle)                              = 0;
        virtual void setImmediatelyNg(bool immediately_ng)            = 0;
    };
    class IDetectComponent
    {
    public:
        virtual ~IDetectComponent()             = default;
        virtual const std::string& name() const = 0;
        virtual const std::string& code() const = 0;
        virtual bool isAiOk() const             = 0;

        virtual std::vector<const IDetectWindow*> windows() const                   = 0;
        virtual const IDetectWindow* findWindow(const std::string& win_name,
                                                const std::string& win_light) const = 0;
        virtual IDetectWindow* findWindow(const std::string& win_name,
                                          const std::string& win_light)             = 0;

        virtual IDetectWindow* mutableWindow(const std::string& win_name,
                                             const std::string& win_light) = 0;
    };
    class IDetectBoard
    {
    public:
        virtual ~IDetectBoard()               = default;
        virtual const std::string& sn() const = 0;
        virtual int index() const             = 0;

        virtual std::vector<const IDetectComponent*> components() const                   = 0;
        virtual const IDetectComponent* findComponent(const std::string& comp_name) const = 0;
        virtual IDetectComponent* findComponent(const std::string& comp_name)             = 0;

        virtual const IDetectWindow* findWindow(const std::string& comp_name,
                                                const std::string& win_name,
                                                const std::string& win_light) const = 0;
        virtual IDetectComponent* mutableComponent(const std::string& comp_name,
                                                   const std::string& comp_code)    = 0;
    };

    class IDetectPanel
    {
    public:
        virtual ~IDetectPanel()                             = default;
        virtual void setLine(const std::string& line)       = 0;
        virtual void setStation(const std::string& station) = 0;
        virtual void setName(const std::string& name)       = 0;
        virtual void setSn(const std::string& sn)           = 0;
        virtual void setEnabledTrustMode(bool enabled)      = 0;

        virtual void setComponentOpResult(int board_index,
                                          const std::string& comp_name,
                                          bool is_ok) = 0;
        virtual void setWindowOpResult(int board_index,
                                       const std::string& comp_name,
                                       const std::string& win_name,
                                       const std::string& win_light,
                                       bool is_ok)    = 0;

        virtual const std::string& line() const    = 0;
        virtual const std::string& station() const = 0;
        virtual const std::string& name() const    = 0;
        virtual const std::string& sn() const      = 0;
        virtual bool isTrustModeEnabled() const    = 0;

        virtual IDetectBoard* mutableBoard(int board_index, const std::string& board_sn) = 0;


        virtual std::vector<const IDetectBoard*> boards() const = 0;

        virtual const IDetectBoard* findBoard(int board_index) const = 0;
        virtual IDetectBoard* findBoard(int board_index)             = 0;

        virtual const IDetectComponent* findComponent(int board_index,
                                                      const std::string& comp_name) const      = 0;
        virtual IDetectComponent* findComponent(int board_index, const std::string& comp_name) = 0;

        virtual const IDetectWindow* findWindow(int board_index,
                                                const std::string& comp_name,
                                                const std::string& win_name,
                                                const std::string& win_light) const = 0;
        virtual IDetectWindow* findWindow(int board_index,
                                          const std::string& comp_name,
                                          const std::string& win_name,
                                          const std::string& win_light)             = 0;
    };

    virtual ~IAIAgentService() = default;

    virtual std::shared_ptr<IDetectPanel> createDetectPanel() const = 0;
    virtual void detect(std::shared_ptr<IDetectPanel> panel)        = 0;
};
} // namespace service