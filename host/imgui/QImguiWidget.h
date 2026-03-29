#pragma once
#include <QOpenGLWidget>

struct ImGuiContext;

class QImguiWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    explicit QImguiWidget(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    ~QImguiWidget();

protected:
    void initializeGL() override;
    void paintGL() override;
    virtual void drawImgui() = 0;

private:
    ImGuiContext* m_ctx {};
};