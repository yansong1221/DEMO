#pragma once
#include "imgui/QImguiWidget.h"
#include "service/IGreetingService.h"
#include <QDialog>

class TaskServiceConfigWidget : public QImguiWidget
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