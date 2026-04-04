#include "MainImGuiWidget.h"

#include "imgui.h"
#include <QCheckBox>
#include <QCoreApplication>
#include <QDateTime>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollBar>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextEdit>
#include <QVBoxLayout>

MainImGuiWidget::MainImGuiWidget(cppmicroservices::BundleContext const& bundleContext, QWidget* parent)
    : ImguiWidget(parent)
    , tracker_(bundleContext, this)
    , bundleContext_(bundleContext)
{
    tracker_.Open();
    imguiWS.init(5000, QCoreApplication::applicationDirPath().toStdString(), { "", ".html" });
}

MainImGuiWidget::~MainImGuiWidget() { tracker_.Close(); }

void
MainImGuiWidget::drawImGui()
{
    for (auto const& ser : services_)
    {
        ser->drawImGui();
    }
}

void
MainImGuiWidget::initializeGL()
{
    ImguiWidget::initializeGL();

    ImGui::StyleColorsDark();

    // ImGui::CreateContext();
    ImGui::GetIO().MouseDrawCursor = true;

    ImGui::StyleColorsDark();
    ImGui::GetStyle().AntiAliasedFill = false;
    ImGui::GetStyle().AntiAliasedLines = false;
    ImGui::GetStyle().WindowRounding = 0.0f;
    ImGui::GetStyle().ScrollbarRounding = 0.0f;
    // prepare font texture
    {
        unsigned char* pixels;
        int width, height;
        ImGui::GetIO().Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);
        imguiWS.setTexture(0, ImGuiWS::Texture::Type::Alpha8, width, height, (char const*)pixels);
    }
}

void
MainImGuiWidget::endDrawImGui()
{
    imguiWS.setDrawData(ImGui::GetDrawData());
}

std::shared_ptr<service::IImGuiDrawService>
MainImGuiWidget::AddingService(cppmicroservices::ServiceReference<service::IImGuiDrawService> const& reference)
{
    auto ser = bundleContext_.GetService(reference);
    if (!ser)
    {
        return nullptr;
    }
    services_.push_back(ser);
    return ser;
}

void
MainImGuiWidget::ModifiedService(cppmicroservices::ServiceReference<service::IImGuiDrawService> const& reference,
                                 std::shared_ptr<service::IImGuiDrawService> const& service)
{
}

void
MainImGuiWidget::RemovedService(cppmicroservices::ServiceReference<service::IImGuiDrawService> const& reference,
                                std::shared_ptr<service::IImGuiDrawService> const& service)
{

    if (auto iter = std::find(services_.begin(), services_.end(), service); iter != services_.end())
    {
        services_.erase(iter);
    }
}
