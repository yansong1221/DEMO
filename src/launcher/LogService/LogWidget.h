#pragma once

#include "ImguiWidget.h"
#include <QTextCharFormat>
#include <QWidget>
#include <memory>
#include <mutex>
#include <spdlog/spdlog.h>
#include <vector>
#include "imguial_term.h"


class LogWidget : public ImguiWidget
{
    Q_OBJECT

  public:
    explicit LogWidget(QWidget* parent = nullptr);
    ~LogWidget() override;

  public Q_SLOTS:
    void addLog(spdlog::level::level_enum level, QString const& message);

  protected:
    void drawImgui() override;
    void initializeGL() override;
  private:
    ImGuiAl::Log _terminal;
};
