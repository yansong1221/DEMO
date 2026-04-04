#pragma once

#include "ImguiWidget.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/ServiceTracker.h"
#include "cppmicroservices/ServiceTrackerCustomizer.h"
#include "imgui-ws/imgui-ws.h"
#include "service/IUIService.h"
#include <QTextCharFormat>
#include <QWidget>
#include <memory>
#include <mutex>
#include <vector>

class MainImGuiWidget
    : public ImguiWidget
    , public cppmicroservices::ServiceTrackerCustomizer<service::IImGuiDrawService>
{
    Q_OBJECT

  public:
    explicit MainImGuiWidget(cppmicroservices::BundleContext const& bundleContext, QWidget* parent = nullptr);
    ~MainImGuiWidget() override;

  protected:
    void initializeGL() override;

    void drawImGui() override;
    void endDrawImGui() override;

  protected:
    std::shared_ptr<service::IImGuiDrawService> AddingService(
        cppmicroservices::ServiceReference<service::IImGuiDrawService> const& reference) override;

    void ModifiedService(cppmicroservices::ServiceReference<service::IImGuiDrawService> const& reference,
                         std::shared_ptr<service::IImGuiDrawService> const& service) override;

    void RemovedService(cppmicroservices::ServiceReference<service::IImGuiDrawService> const& reference,
                        std::shared_ptr<service::IImGuiDrawService> const& service) override;

  private:
    ImGuiWS imguiWS;
    cppmicroservices::BundleContext bundleContext_;
    cppmicroservices::ServiceTracker<service::IImGuiDrawService> tracker_;
    std::vector<std::shared_ptr<service::IImGuiDrawService>> services_;
};
