#include "TaskServiceConfigDialog.h"
#include "imgui.h"
#include <QDialogButtonBox>
#include <QVBoxLayout>

TaskServiceConfigWidget::TaskServiceConfigWidget(std::shared_ptr<service::ITaskService::IBasicConfig> config,
                                                 QWidget* parent /*= nullptr*/)
    : ImguiWidget(parent)
    , config_(config)
{
}

TaskServiceConfigWidget::~TaskServiceConfigWidget() {}

void
TaskServiceConfigWidget::drawImgui()
{
    config_->draw(ImGui::GetCurrentContext());
}

TaskServiceConfigDialog::TaskServiceConfigDialog(std::shared_ptr<service::ITaskService::IBasicConfig> config,
                                                 QWidget* parent /*= nullptr*/)
    : QDialog(parent)
{
    auto content = new TaskServiceConfigWidget(config, this);

    QVBoxLayout* layout = new QVBoxLayout(this);

    layout->addWidget(content);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    QObject::connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    layout->addWidget(buttonBox);
}
