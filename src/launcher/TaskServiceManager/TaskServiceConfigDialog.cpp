#include "TaskServiceConfigDialog.h"
#include "imgui.h"
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QDebug>

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
    config_->draw();
}

TaskServiceConfigDialog::TaskServiceConfigDialog(std::shared_ptr<service::ITaskService::IBasicConfig> config,
                                                 QWidget* parent /*= nullptr*/)
    : QDialog(parent)
{
    auto content = new TaskServiceConfigWidget(config, this);
    content->setFocusPolicy(Qt::StrongFocus);

    QVBoxLayout* layout = new QVBoxLayout(this);

    layout->addWidget(content);
    //layout->addStretch();

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);

    QObject::connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    layout->addWidget(buttonBox);
    this->resize(800, 600);
}

void
TaskServiceConfigDialog::keyPressEvent(QKeyEvent* event)
{
    qDebug() << "key press event: " << event->text();
}
