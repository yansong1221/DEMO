#include "MainWindow.h"
#include "PluginBundleActionDelegate.h"
#include "PluginBundleTableModel.h"
#include "TaskServiceActionDelegate.h"
#include "TaskServiceTableModel.h"
#include "ui_MainWindow.h"

#include <QAbstractItemView>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QHeaderView>
#include <QInputDialog>
#include <QItemSelectionModel>
#include <QMessageBox>

#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/FrameworkFactory.h>

#include <chrono>
#include <exception>
#include <string>

#include <QDockWidget>

namespace {

std::string toUtf8StdString(QString const& value)
{
    const QByteArray utf8 = value.toUtf8();
    return std::string(utf8.constData(), static_cast<size_t>(utf8.size()));
}

} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_framework(cppmicroservices::FrameworkFactory {}.NewFramework())
{
    ui->setupUi(this);

    m_framework.Init();
    m_framework.Start();

    m_ListenerToken = m_framework.GetBundleContext().AddServiceListener(
        [this](cppmicroservices::ServiceEvent const& evt) {
            auto type     = evt.GetType();
            auto eventRef = evt.GetServiceReference();

            if (type & cppmicroservices::ServiceEvent::SERVICE_REGISTERED) {
                auto ctx     = m_framework.GetBundleContext();
                auto service = ctx.GetService<demo::IWidgetService>(eventRef);
                if (!service) {
                    return;
                }
                auto w = service->createWidget(this);

                auto dock_W = new QDockWidget(tr("插件界面"), this);
                dock_W->setWidget(w);

                this->addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, dock_W);

                PluginState p;
                p.service = service;
                p.ref     = eventRef;
                p.dock_w  = dock_W;
                m_Plugins.push_back(p);
            }
            else if (type & cppmicroservices::ServiceEvent::SERVICE_UNREGISTERING) {
                for (int i = 0; i < m_Plugins.size(); ++i) {
                    const cppmicroservices::ServiceReferenceBase currentRef = m_Plugins[i].ref;
                    auto service                                            = m_Plugins[i].service;
                    auto dock_w                                             = m_Plugins[i].dock_w;

                    if (currentRef == eventRef) {
                        service->destroyWidget(dock_w->widget());
                        this->removeDockWidget(dock_w);
                        m_Plugins.erase(m_Plugins.begin() + i);
                        break;
                    }
                }
            }
        });

    setupModels();
    setupViews();
    setupConnections();

    // 设置初始目录
    ui->pluginDirEdit->setText(QCoreApplication::applicationDirPath());

    appendLog(tr("框架已启动，支持 Bundle 管理和 ITaskService 服务发现。"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupModels()
{
    m_bundleModel = new PluginBundleTableModel(this);
    connect(m_bundleModel, &PluginBundleTableModel::bundleLog, this, &MainWindow::appendLog);
    m_bundleModel->setHostFramework(&m_framework);

    m_taskServiceModel = new TaskServiceTableModel(this);
    connect(m_taskServiceModel,
            &TaskServiceTableModel::serviceLog,
            this,
            &MainWindow::onTaskServiceLog);
    m_taskServiceModel->setFramework(&m_framework);
}

void MainWindow::setupViews()
{
    // Bundle 表格设置
    ui->bundleView->setModel(m_bundleModel);
    ui->bundleView->horizontalHeader()->setStretchLastSection(false);
    ui->bundleView->horizontalHeader()->setSectionResizeMode(PluginBundleTableModel::ColDescription,
                                                             QHeaderView::Stretch);
    ui->bundleView->horizontalHeader()->setSectionResizeMode(PluginBundleTableModel::ColActions,
                                                             QHeaderView::Fixed);
    ui->bundleView->setColumnWidth(PluginBundleTableModel::ColActions, 90);

    m_actionDelegate = new PluginBundleActionDelegate(ui->bundleView);
    ui->bundleView->setItemDelegateForColumn(PluginBundleTableModel::ColActions, m_actionDelegate);

    // 任务服务表格设置
    ui->taskServiceView->setModel(m_taskServiceModel);
    ui->taskServiceView->horizontalHeader()->setStretchLastSection(false);
    ui->taskServiceView->horizontalHeader()->setSectionResizeMode(TaskServiceTableModel::ColName,
                                                                  QHeaderView::Stretch);
    // ui->taskServiceView->horizontalHeader()->setSectionResizeMode(TaskServiceTableModel::ColBundle,
    // QHeaderView::Stretch);
    // ui->taskServiceView->horizontalHeader()->setSectionResizeMode(TaskServiceTableModel::ColStatus,
    // QHeaderView::ResizeToContents);
    ui->taskServiceView->horizontalHeader()->setSectionResizeMode(TaskServiceTableModel::ColActions,
                                                                  QHeaderView::Fixed);
    ui->taskServiceView->setColumnWidth(TaskServiceTableModel::ColActions, 115);

    m_taskServiceActionDelegate = new TaskServiceActionDelegate(ui->taskServiceView);
    ui->taskServiceView->setItemDelegateForColumn(TaskServiceTableModel::ColActions,
                                                  m_taskServiceActionDelegate);

    // 设置 splitter 拉伸因子
    ui->bundleSplitter->setStretchFactor(0, 3);
    ui->bundleSplitter->setStretchFactor(1, 2);

    // 日志区域最大高度
    ui->logEdit->setMaximumHeight(180);
}

void MainWindow::setupConnections()
{
    // Bundle 相关连接
    connect(ui->browseDirBtn, &QPushButton::clicked, this, &MainWindow::onBrowsePluginFolder);
    connect(ui->refreshListBtn, &QPushButton::clicked, this, &MainWindow::onRefreshBundleList);
    connect(ui->bundleView->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            &MainWindow::onBundleTableSelectionChanged);
    connect(m_actionDelegate,
            &PluginBundleActionDelegate::loadRequested,
            this,
            &MainWindow::onLoadBundleRow);
    connect(m_actionDelegate,
            &PluginBundleActionDelegate::unloadRequested,
            this,
            &MainWindow::onUnloadBundleRow);

    // 任务服务相关连接
    connect(
        ui->refreshServicesBtn, &QPushButton::clicked, this, &MainWindow::onRefreshTaskServices);
    connect(m_taskServiceActionDelegate,
            &TaskServiceActionDelegate::startStopRequested,
            this,
            &MainWindow::onTaskServiceStartStopRequested);
    connect(m_taskServiceActionDelegate,
            &TaskServiceActionDelegate::configRequested,
            this,
            &MainWindow::onTaskServiceConfigRequested);

    // 启动监听器
    if (!m_bundleModel->attachBundleListener()) {
        appendLog(tr("[监听] 在模型中注册 Bundle 监听器失败。"));
    }

    if (!m_taskServiceModel->attachListener()) {
        appendLog(tr("[监听] 在模型中注册任务服务监听器失败。"));
    }

    m_taskServiceModel->refresh();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    for (auto& bundle : m_framework.GetBundleContext().GetBundles()) {
        bundle.Stop();
    }
    m_bundleModel->detachBundleListener();
    m_taskServiceModel->detachListener();

    m_framework.GetBundleContext().RemoveListener(std::move(m_ListenerToken));

    m_framework.Stop();
    m_framework.WaitForStop((std::chrono::milliseconds::max)());
    QMainWindow::closeEvent(event);
}

void MainWindow::appendLog(const QString& line)
{
    ui->logEdit->append(line);
}

void MainWindow::onTaskServiceLog(QString const& message)
{
    appendLog(message);
}

int MainWindow::currentTaskServiceRow() const
{
    if (!ui->taskServiceView || !ui->taskServiceView->selectionModel()) {
        return -1;
    }
    const QModelIndex cur = ui->taskServiceView->selectionModel()->currentIndex();
    if (!cur.isValid()) {
        return -1;
    }
    return cur.row();
}

std::shared_ptr<demo::ITaskService::IBasicConfig> MainWindow::buildTaskServiceConfig(int row,
                                                                                     bool* ok) const
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

void MainWindow::onBrowsePluginFolder()
{
    const QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("选择插件目录"),
        ui->pluginDirEdit->text().isEmpty() ? QCoreApplication::applicationDirPath()
                                            : ui->pluginDirEdit->text());
    if (!dir.isEmpty()) {
        ui->pluginDirEdit->setText(QDir::toNativeSeparators(dir));
    }
}

void MainWindow::onRefreshBundleList()
{
    m_bundleModel->rescanPluginDirectory(ui->pluginDirEdit->text());
}

void MainWindow::onRefreshTaskServices()
{
    m_taskServiceModel->refresh();
    appendLog(tr("[任务服务] 已刷新，共发现 %1 个服务。").arg(m_taskServiceModel->rowCount()));
}

void MainWindow::onBundleTableSelectionChanged()
{
    const QModelIndex cur = ui->bundleView->selectionModel()->currentIndex();
    if (!cur.isValid()) {
        return;
    }
}

void MainWindow::onTaskServiceSelectionChanged()
{
    // 选择变更处理（如有需要）
}

void MainWindow::onLoadBundleRow(int row)
{
    m_bundleModel->startBundleRow(row);
}

void MainWindow::onUnloadBundleRow(int row)
{
    const QString symbolicName = m_bundleModel->symbolicNameAtRow(row);
    const QString absPath      = m_bundleModel->absPathAtRow(row);
    const QString bundleName =
        symbolicName.isEmpty() ? QFileInfo(absPath).fileName() : symbolicName;

    const QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        tr("确认卸载 Bundle"),
        tr("确定要卸载 Bundle \"%1\" 吗？\n这会先停止该 Bundle，再将它从当前框架中卸载。")
            .arg(bundleName),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (answer != QMessageBox::Yes) {
        appendLog(tr("[卸载] 已取消：%1").arg(bundleName));
        return;
    }

    m_bundleModel->stopBundleRow(row);
}

void MainWindow::onTaskServiceStartStopRequested(int row)
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

void MainWindow::onTaskServiceConfigRequested(int row)
{
    const TaskServiceEntry* entry = m_taskServiceModel->entryAt(row);
    if (!entry || !entry->service) {
        return;
    }

    bool ok;
    QString configText = QInputDialog::getMultiLineText(
        this,
        tr("编辑配置 - %1").arg(QString::fromStdString(entry->service->name())),
        tr("YAML 配置:"),
        entry->defaultConfigYaml,
        &ok);

    if (!ok) {
        return;
    }

    appendLog(tr("[任务服务] 已更新配置：%1").arg(QString::fromStdString(entry->service->name())));
}

void MainWindow::startTaskService(int row)
{
    if (row < 0) {
        return;
    }

    TaskServiceEntry* entry = m_taskServiceModel->entryAt(row);
    if (!entry || !entry->service) {
        return;
    }

    // 设置日志输出目标为界面的 logEdit
    entry->setLogTarget(ui->logEdit);

    bool ok           = false;
    const auto config = buildTaskServiceConfig(row, &ok);
    if (!ok) {
        return;
    }

    // 使用线程启动服务
    bool started = entry->startService(config);
    if (started) {
        appendLog(tr("[任务服务] %1 启动成功（线程已启动）。")
                      .arg(QString::fromStdString(entry->service->name())));
    }
    else {
        appendLog(
            tr("[任务服务] %1 启动失败。").arg(QString::fromStdString(entry->service->name())));
    }

    // 触发数据更新以刷新按钮状态
    emit m_taskServiceModel->dataChanged(
        m_taskServiceModel->index(row, TaskServiceTableModel::ColStatus),
        m_taskServiceModel->index(row, TaskServiceTableModel::ColActions));
}

void MainWindow::stopTaskService(int row)
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
        appendLog(tr("[任务服务] %1 已停止（线程已结束）。")
                      .arg(QString::fromStdString(entry->service->name())));
    }
    else {
        appendLog(
            tr("[任务服务] %1 停止超时。").arg(QString::fromStdString(entry->service->name())));
    }

    // 触发数据更新以刷新按钮状态
    emit m_taskServiceModel->dataChanged(
        m_taskServiceModel->index(row, TaskServiceTableModel::ColStatus),
        m_taskServiceModel->index(row, TaskServiceTableModel::ColActions));
}
