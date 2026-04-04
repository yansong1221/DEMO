#pragma once
#include <boost/asio/awaitable.hpp>
#include <opencv2/opencv.hpp>
#include <set>
#include <vector>

namespace service
{

    class IAIAgentService
    {
      public:
        class IDetectWindow
        {
          public:
            virtual ~IDetectWindow() = default;
            virtual std::string const& name() const = 0;
            virtual std::string const& light() const = 0;
            virtual bool isImmediatelyNg() const = 0;
            virtual bool isAiOk() const = 0;

            virtual void setImage(cv::Mat const& image) = 0;
            virtual void setBase64Image(std::string&& base64_image) = 0;
            virtual void setFileImage(std::string const& file_path) = 0;

            virtual void setDefects(std::set<std::string> const& defects) = 0;
            virtual void setDetectRois(std::vector<cv::Rect> const& rois) = 0;
            virtual void setTrainRois(std::vector<cv::Rect> const& rois) = 0;
            virtual void setAngle(int angle) = 0;
            virtual void setImmediatelyNg(bool immediately_ng) = 0;
        };
        class IDetectComponent
        {
          public:
            virtual ~IDetectComponent() = default;
            virtual std::string const& name() const = 0;
            virtual std::string const& code() const = 0;
            virtual bool isAiOk() const = 0;

            virtual std::vector<IDetectWindow const*> windows() const = 0;
            virtual IDetectWindow const* findWindow(std::string const& win_name, std::string const& win_light) const
                = 0;
            virtual IDetectWindow* findWindow(std::string const& win_name, std::string const& win_light) = 0;

            virtual IDetectWindow* mutableWindow(std::string const& win_name, std::string const& win_light) = 0;
        };
        class IDetectBoard
        {
          public:
            virtual ~IDetectBoard() = default;
            virtual std::string const& sn() const = 0;
            virtual int index() const = 0;

            virtual std::vector<IDetectComponent const*> components() const = 0;
            virtual IDetectComponent const* findComponent(std::string const& comp_name) const = 0;
            virtual IDetectComponent* findComponent(std::string const& comp_name) = 0;

            virtual IDetectWindow const* findWindow(std::string const& comp_name,
                                                    std::string const& win_name,
                                                    std::string const& win_light) const
                = 0;
            virtual IDetectComponent* mutableComponent(std::string const& comp_name, std::string const& comp_code) = 0;
        };

        class IDetectPanel
        {
          public:
            virtual ~IDetectPanel() = default;
            virtual void setLine(std::string const& line) = 0;
            virtual void setStation(std::string const& station) = 0;
            virtual void setName(std::string const& name) = 0;
            virtual void setSn(std::string const& sn) = 0;
            virtual void setEnabledTrustMode(bool enabled) = 0;

            virtual void setComponentOpResult(int board_index, std::string const& comp_name, bool is_ok) = 0;
            virtual void setWindowOpResult(int board_index,
                                           std::string const& comp_name,
                                           std::string const& win_name,
                                           std::string const& win_light,
                                           bool is_ok)
                = 0;

            virtual std::string const& line() const = 0;
            virtual std::string const& station() const = 0;
            virtual std::string const& name() const = 0;
            virtual std::string const& sn() const = 0;
            virtual bool isTrustModeEnabled() const = 0;

            virtual IDetectBoard* mutableBoard(int board_index, std::string const& board_sn) = 0;

            virtual std::vector<IDetectBoard const*> boards() const = 0;

            virtual IDetectBoard const* findBoard(int board_index) const = 0;
            virtual IDetectBoard* findBoard(int board_index) = 0;

            virtual IDetectComponent const* findComponent(int board_index, std::string const& comp_name) const = 0;
            virtual IDetectComponent* findComponent(int board_index, std::string const& comp_name) = 0;

            virtual IDetectWindow const* findWindow(int board_index,
                                                    std::string const& comp_name,
                                                    std::string const& win_name,
                                                    std::string const& win_light) const
                = 0;
            virtual IDetectWindow* findWindow(int board_index,
                                              std::string const& comp_name,
                                              std::string const& win_name,
                                              std::string const& win_light)
                = 0;
        };

        virtual ~IAIAgentService() = default;

        virtual std::shared_ptr<IDetectPanel> createDetectPanel() const = 0;
        virtual void detect(std::shared_ptr<IDetectPanel> panel) = 0;
        virtual boost::asio::awaitable<void> coroDetect(std::shared_ptr<IDetectPanel> panel) = 0;

        virtual bool isTrustProgram(std::string const& line, std::string const& station, std::string const& name) = 0;
    };
} // namespace service