#include "TaskServiceDockWidget.h"
#include "TaskServiceActionDelegate.h"
#include "TaskServiceConfigDialog.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QItemSelectionModel>
#include <QPushButton>
#include <QSpacerItem>
#include <QTableView>
#include <QTextEdit>
#include <QVBoxLayout>

#include "common/Logger.h"
#include <cppmicroservices/BundleContext.h>

TaskServiceDockWidget::TaskServiceDockWidget(cppmicroservices::BundleContext bundleContext,
                                             QWidget* parent)
    : KDDockWidgets::QtWidgets::DockWidget(QStringLiteral("TaskServiceManager"))
    , m_bundleContext(bundleContext)
{
    Q_UNUSED(parent)
    setupUI();
    setupConnections();

    // 启动监听器
    if (!m_taskServiceModel->attachListener()) {
        common::Logger::error(tr("[监听] 在模型中注册任务服务监听器失败。").toStdString());
    }

    m_taskServiceModel->refresh();
}

TaskServiceDockWidget::~TaskServiceDockWidget()
{
    m_taskServiceModel->detachListener();
}

void TaskServiceDockWidget::setupUI()
{
    auto* centralWidget = new QWidget(this);
    auto* mainLayout    = new QVBoxLayout(centralWidget);
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
    m_taskServiceView->horizontalHeader()->setSectionResizeMode(TaskServiceTableModel::ColName,
                                                                QHeaderView::Stretch);
    m_taskServiceView->horizontalHeader()->setSectionResizeMode(TaskServiceTableModel::ColActions,
                                                                QHeaderView::Fixed);
    m_taskServiceView->setColumnWidth(TaskServiceTableModel::ColActions, 115);

    m_taskServiceActionDelegate = new TaskServiceActionDelegate(m_taskServiceView);
    m_taskServiceView->setItemDelegateForColumn(TaskServiceTableModel::ColActions,
                                                m_taskServiceActionDelegate);

    mainLayout->addWidget(m_taskServiceView);

    setWidget(centralWidget);
    setTitle(tr("任务服务"));
}

void TaskServiceDockWidget::setupConnections()
{
    connect(m_refreshServicesBtn,
            &QPushButton::clicked,
            this,
            &TaskServiceDockWidget::onRefreshTaskServices);
    connect(m_taskServiceActionDelegate,
            &TaskServiceActionDelegate::startStopRequested,
            this,
            &TaskServiceDockWidget::onTaskServiceStartStopRequested);
    connect(m_taskServiceActionDelegate,
            &TaskServiceActionDelegate::configRequested,
            this,
            &TaskServiceDockWidget::onTaskServiceConfigRequested);
}

void TaskServiceDockWidget::refreshTaskServices()
{
    m_taskServiceModel->refresh();
    common::Logger::info(tr("[任务服务] 已刷新，共发现 %1 个服务。")
                             .arg(m_taskServiceModel->rowCount())
                             .toStdString());
}

void TaskServiceDockWidget::startTaskService(int row)
{
    if (row < 0) {
        return;
    }

    TaskServiceEntry* entry = m_taskServiceModel->entryAt(row);
    if (!entry || !entry->service) {
        return;
    }

    bool ok           = false;
    const auto config = buildTaskServiceConfig(row, &ok);
    if (!ok) {
        return;
    }

    // 使用线程启动服务
    bool started = entry->startService(config);
    if (started) {
        common::Logger::info(tr("[任务服务] %1 启动成功（线程已启动）。")
                                 .arg(QString::fromStdString(entry->service->name()))
                                 .toStdString());
    }
    else {
        common::Logger::warn(tr("[任务服务] %1 启动失败。")
                                 .arg(QString::fromStdString(entry->service->name()))
                                 .toStdString());
    }

    // 触发数据更新以刷新按钮状态
    emit m_taskServiceModel->dataChanged(
        m_taskServiceModel->index(row, TaskServiceTableModel::ColStatus),
        m_taskServiceModel->index(row, TaskServiceTableModel::ColActions));

    emit taskServiceStarted(row);
}

void TaskServiceDockWidget::stopTaskService(int row)
{
    if (row < 0) {
        return;
    }

    TaskServiceEntry* entry = m_taskServiceModel->entryAt(row);
    if (!entry || !entry->service) {
        return;
    }

    // 使用线程停止服务
    bool stopped = entry->stopService(5000); // 5秒超时
    if (stopped) {
        common::Logger::info(tr("[任务服务] %1 已停止（线程已结束）。")
                                 .arg(QString::fromStdString(entry->service->name()))
                                 .toStdString());
    }
    else {
        common::Logger::error(tr("[任务服务] %1 停止超时。")
                                  .arg(QString::fromStdString(entry->service->name()))
                                  .toStdString());
    }

    // 触发数据更新以刷新按钮状态
    emit m_taskServiceModel->dataChanged(
        m_taskServiceModel->index(row, TaskServiceTableModel::ColStatus),
        m_taskServiceModel->index(row, TaskServiceTableModel::ColActions));

    emit taskServiceStopped(row);
}

void TaskServiceDockWidget::configureTaskService(int row)
{
    const TaskServiceEntry* entry = m_taskServiceModel->entryAt(row);
    if (!entry || !entry->service) {
        return;
    }

    TaskServiceConfigDialog dialog(entry->service->createYamlConfig(), this);
    dialog.resize(800, 600);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    common::Logger::info(tr("[任务服务] 已更新配置：%1")
                             .arg(QString::fromStdString(entry->service->name()))
                             .toStdString());
    emit taskServiceConfigured(row);
}

int TaskServiceDockWidget::currentTaskServiceRow() const
{
    if (!m_taskServiceView || !m_taskServiceView->selectionModel()) {
        return -1;
    }
    const QModelIndex cur = m_taskServiceView->selectionModel()->currentIndex();
    if (!cur.isValid()) {
        return -1;
    }
    return cur.row();
}

std::shared_ptr<service::ITaskService::IBasicConfig>
TaskServiceDockWidget::buildTaskServiceConfig(int row, bool* ok) const
{
    if (ok) {
        *ok = false;
    }

    if (row < 0) {
        return {};
    }

    const TaskServiceEntry* entry = m_taskServiceModel->entryAt(row);
    if (!entry || !entry->service) {
        return {};
    }

    auto config = entry->service->createYamlConfig();
    if (!config) {
        if (ok) {
            *ok = true;
        }
        return {};
    }

    if (ok) {
        *ok = true;
    }
    return config;
}

void TaskServiceDockWidget::onRefreshTaskServices()
{
    refreshTaskServices();
}

void TaskServiceDockWidget::onTaskServiceStartStopRequested(int row)
{
    TaskServiceEntry* entry = m_taskServiceModel->entryAt(row);
    if (!entry || !entry->service) {
        return;
    }

    if (entry->isRunning()) {
        stopTaskService(row);
    }
    else {
        startTaskService(row);
    }
}

void TaskServiceDockWidget::onTaskServiceConfigRequested(int row)
{
    configureTaskService(row);
}