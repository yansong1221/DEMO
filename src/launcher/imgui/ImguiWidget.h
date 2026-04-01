#pragma once
#include <QOpenGLWidget>

struct ImGuiContext;

class ImguiWidget : public QOpenGLWidget
{
    Q_OBJECT
  public:
    explicit ImguiWidget(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    ~ImguiWidget();

  protected:
    void initializeGL() override;
    void paintGL() override;
    virtual void drawImgui() = 0;

  private:
    ImGuiContext* m_ctx {};
};