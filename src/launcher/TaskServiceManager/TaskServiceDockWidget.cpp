#include "TaskServiceDockWidget.h"
#include "TaskServiceActionDelegate.h"
#include "TaskServiceConfigDialog.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QPushButton>
#include <QSpacerItem>
#include <QTableView>
#include <QTextEdit>
#include <QVBoxLayout>

#include "common/Logger.h"
#include <cppmicroservices/BundleContext.h>

TaskServiceDockWidget::TaskServiceDockWidget(cppmicroservices::BundleContext bundleContext, QWidget* parent)
    : KDDockWidgets::QtWidgets::DockWidget(QStringLiteral("TaskServiceManager"))
    , m_bundleContext(bundleContext)
{
    Q_UNUSED(parent)
    setupUI();
    setupConnections();

    // 启动监听器
    if (!m_taskServiceModel->attachListener())
    {
        common::Log::error(tr("[监听] 在模型中注册任务服务监听器失败。").toStdString());
    }

    m_taskServiceModel->refresh();
}

TaskServiceDockWidget::~TaskServiceDockWidget() { m_taskServiceModel->detachListener(); }

void
TaskServiceDockWidget::setupUI()
{
    auto* centralWidget = new QWidget(this);
    auto* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);

    // 工具栏
    auto* toolbarLayout = new QHBoxLayout();
    toolbarLayout->setSpacing(4);

    m_refreshServicesBtn = new QPushButton(tr("刷新服务"), this);
    toolbarLayout->addWidget(m_refreshServicesBtn);
    toolbarLayout->addStretch();

    mainLayout->addLayout(toolbarLayout);

    // 任务服务表格
    m_taskServiceView = new QTableView(this);
    m_taskServiceView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_taskServiceView->setAlternatingRowColors(true);
    m_taskServiceView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_taskServiceView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_taskServiceView->verticalHeader()->setVisible(false);
    m_taskServiceView->horizontalHeader()->setStretchLastSection(true);

    m_taskServiceModel = new TaskServiceTableModel(m_bundleContext, this);
    m_taskServiceView->setModel(m_taskServiceModel);
    m_taskServiceView->horizontalHeader()->setStretchLastSection(false);
    m_taskServiceView->horizontalHeader()->setSectionResizeMode(TaskServiceTableModel::ColName, QHeaderView::Stretch);
    m_taskServiceView->horizontalHeader()->setSectionResizeMode(TaskServiceTableModel::ColActions, QHeaderView::Fixed);
    m_taskServiceView->setColumnWidth(TaskServiceTableModel::ColActions, 115);

    m_taskServiceActionDelegate = new TaskServiceActionDelegate(m_taskServiceView);
    m_taskServiceView->setItemDelegateForColumn(TaskServiceTableModel::ColActions, m_taskServiceActionDelegate);

    mainLayout->addWidget(m_taskServiceView);

    setWidget(centralWidget);
    setTitle(tr("任务服务"));
}

void
TaskServiceDockWidget::setupConnections()
{
    connect(m_refreshServicesBtn, &QPushButton::clicked, this, &TaskServiceDockWidget::onRefreshTaskServices);
    connect(m_taskServiceActionDelegate,
            &TaskServiceActionDelegate::startStopRequested,
            this,
            &TaskServiceDockWidget::onTaskServiceStartStopRequested);
    connect(m_taskServiceActionDelegate,
            &TaskServiceActionDelegate::configRequested,
            this,
            &TaskServiceDockWidget::onTaskServiceConfigRequested);
}

void
TaskServiceDockWidget::refreshTaskServices()
{
    m_taskServiceModel->refresh();
    common::Log::info(tr("[任务服务] 已刷新，共发现 %1 个服务。").arg(m_taskServiceModel->rowCount()).toStdString());
}

void
TaskServiceDockWidget::startTaskService(int row)
{
    if (row < 0)
    {
        return;
    }

    // 直接调用模型的startService方法
    bool started = m_taskServiceModel->startService(row);
    if (started)
    {
        common::Log::info(tr("[任务服务] 服务 %1 启动成功。").arg(row + 1).toStdString());
    }
    else
    {
        common::Log::warn(tr("[任务服务] 服务 %1 启动失败。").arg(row + 1).toStdString());
    }

    // emit taskServiceStarted(row);
}

void
TaskServiceDockWidget::stopTaskService(int row)
{
    if (row < 0)
    {
        return;
    }

    // 直接调用模型的stopService方法
    m_taskServiceModel->stopService(row);
    common::Log::info(tr("[任务服务] 服务 %1 已停止。").arg(row + 1).toStdString());

    // emit taskServiceStopped(row);
}

void
TaskServiceDockWidget::configureTaskService(int row)
{
    if (row < 0)
    {
        return;
    }
    auto config = m_taskServiceModel->createConfig(row);

    TaskServiceConfigDialog dialog(config, this);
    if (dialog.exec() != QDialog::Accepted)
    {
        return; // 用户取消了配置，直接返回
    }
    m_taskServiceModel->configureService(row, config);

    if (m_taskServiceModel->isRunning(row))
    {
        if (QMessageBox::question(this, tr("询问"), tr("是否重启服务?")) == QMessageBox::Yes)
        {
            m_taskServiceModel->stopService(row);
            m_taskServiceModel->startService(row);
        }
    }
}

int
TaskServiceDockWidget::currentTaskServiceRow() const
{
    if (!m_taskServiceView || !m_taskServiceView->selectionModel())
    {
        return -1;
    }
    QModelIndex const cur = m_taskServiceView->selectionModel()->currentIndex();
    if (!cur.isValid())
    {
        return -1;
    }
    return cur.row();
}

std::shared_ptr<service::ITaskService::IBasicConfig>
TaskServiceDockWidget::buildTaskServiceConfig(int row, bool* ok) const
{
    if (ok)
    {
        *ok = false;
    }

    if (row < 0)
    {
        return {};
    }

    if (ok)
    {
        *ok = true;
    }
    return {};
}

void
TaskServiceDockWidget::onRefreshTaskServices()
{
    refreshTaskServices();
}

void
TaskServiceDockWidget::onTaskServiceStartStopRequested(int row)
{
    if (row < 0)
    {
        return;
    }

    if (m_taskServiceModel->isRunning(row))
    {
        stopTaskService(row);
    }
    else
    {
        startTaskService(row);
    }
}

void
TaskServiceDockWidget::onTaskServiceConfigRequested(int row)
{
    configureTaskService(row);
}