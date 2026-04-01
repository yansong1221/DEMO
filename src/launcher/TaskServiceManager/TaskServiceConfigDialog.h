#pragma once
#include "imgui/ImguiWidget.h"
#include "service/ITaskService.h"
#include <QDialog>

class TaskServiceConfigWidget : public ImguiWidget
{
    Q_OBJECT
  public:
    explicit TaskServiceConfigWidget(std::shared_ptr<service::ITaskService::IBasicConfig> config,
                                     QWidget* parent = nullptr);
    ~TaskServiceConfigWidget();

  protected:
    void drawImgui() override;

  private:
    std::shared_ptr<service::ITaskService::IBasicConfig> config_;
};

class TaskServiceConfigDialog : public QDialog
{
    Q_OBJECT
  public:
    explicit TaskServiceConfigDialog(std::shared_ptr<service::ITaskService::IBasicConfig> config,
                                     QWidget* parent = nullptr);
};