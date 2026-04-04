#pragma once
#include "imgui_extend/QtImGui.h"
#include <QOpenGLExtraFunctions>
#include <QOpenGLWidget>

struct ImGuiContext;

class ImguiWidget
    : public QOpenGLWidget
    , private QOpenGLExtraFunctions
{
    Q_OBJECT
  public:
    explicit ImguiWidget(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    ~ImguiWidget();

  protected:
    void initializeGL() override;
    void paintGL() override;
    virtual void drawImGui() = 0;
    virtual void
    endDrawImGui()
    {
    }

  protected:
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;

  private:
    QtImGui::RenderRef ref_ = nullptr;
    QTimer* updateTimer_;
};