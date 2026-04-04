#include "ImguiWidget.h"
#include "imgui.h"
#include <QApplication>
#include <QDebug>
#include <QScreen>
#include <QTimer>
#include <filesystem>

static float
getQtDpiScale()
{
    QScreen* screen = QGuiApplication::primaryScreen();
    if (!screen)
    {
        return 1.0f;
    }

    return screen->devicePixelRatio();
}
static float
QtFontToPixelSize(QFont const& font)
{
    QFontInfo info(font);

    if (info.pixelSize() > 0)
    {
        return info.pixelSize();
    }

    float pt = info.pointSizeF();
    if (pt <= 0)
    {
        pt = 12.0f;
    }

    float dpi = QGuiApplication::primaryScreen()->logicalDotsPerInch();

    return pt * dpi / 72.0f;
}
static void
SetupFontWithQt()
{
    ImGuiIO& io = ImGui::GetIO();
    float pixelSize = QtFontToPixelSize(QApplication::font());

    io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/msyh.ttc", 20.f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
}

ImguiWidget::ImguiWidget(QWidget* parent /*= Q_NULLPTR*/, Qt::WindowFlags f /*= Qt::WindowFlags()*/)
    : QOpenGLWidget(parent, f)
{
    updateTimer_ = new QTimer(this);
    QObject::connect(updateTimer_, SIGNAL(timeout()), this, SLOT(update()));
    updateTimer_->setInterval(16);
    this->setFocusPolicy(Qt::StrongFocus);
}

ImguiWidget::~ImguiWidget() {}
void
ImguiWidget::initializeGL()
{
    initializeOpenGLFunctions();
    ref_ = QtImGui::initialize(this, false);

    IMGUI_CHECKVERSION();

    ImGuiIO& io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    io.ConfigDpiScaleFonts = true;
    io.ConfigDpiScaleViewports = true;

    float main_scale = getQtDpiScale();
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;

    style.Colors[ImGuiCol_Tab] = ImVec4(0.2f, 0.4f, 0.6f, 1.0f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(0.1f, 0.3f, 0.8f, 1.0f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.1f, 0.3f, 0.8f, 1.0f);

    SetupFontWithQt();
    // if (false)
    //{
    //     namespace fs = std::filesystem;

    //    ImFontGlyphRangesBuilder builder;
    //    builder.AddRanges(io.Fonts->GetGlyphRangesChineseFull());
    //    builder.AddRanges(io.Fonts->GetGlyphRangesVietnamese());

    //    ImVector<ImWchar> out_ranges;
    //    builder.BuildRanges(&out_ranges);

    //    std::vector<fs::path> const fonts {
    //        "C:\\Windows\\Fonts\\msyh.ttc",
    //        //"C:\\Windows\\Fonts\\msyh.ttf",
    //        //"C:\\Windows\\Fonts\\segoeui.ttf",
    //    };
    //    // io.Fonts->AddFontDefault();

    //    QFont font;

    //    ImFontConfig config;
    //    config.MergeMode = true;

    //    for (auto const& p : fonts)
    //    {
    //        std::error_code ec;
    //        if (!fs::exists(p, ec))
    //        {
    //            continue;
    //        }
    //        // io.Fonts->AddFontFromFileTTF(p.string().c_str(), 24.0f, nullptr, out_ranges.Data);

    //        io.Fonts->AddFontFromFileTTF(p.string().c_str(), 24.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());

    //        if (!config.MergeMode)
    //        {
    //            config.MergeMode = true;
    //        }
    //    }
    //    // if (!config.MergeMode) {
    //    //     io.Fonts->AddFontDefault(&config);
    //    // }
    //}
    ImGui::StyleColorsLight();
}

void
ImguiWidget::paintGL()
{
    QtImGui::newFrame(ref_);

    ImGuiViewport const* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    static ImGuiWindowFlags flags = ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoTitleBar
                                    | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings
                                    | ImGuiWindowFlags_NoDecoration;

    // 1. Show a simple window
    // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window
    // automatically called "Debug"
    if (ImGui::Begin("##FullWindowHiddenTitle", nullptr, flags))
    {
        this->drawImGui();
    }
    ImGui::End();

    ImGui::Render();

    endDrawImGui();

    QtImGui::render(ref_);
}

void
ImguiWidget::showEvent(QShowEvent* event)
{
    updateTimer_->start();
}

void
ImguiWidget::hideEvent(QHideEvent* event)
{
    updateTimer_->stop();
}
