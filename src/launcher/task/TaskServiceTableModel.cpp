#include "TaskServiceTableModel.h"

#include <QMetaObject>
#include <QTimer>
#include <cppmicroservices/BundleContext.h>

#include "common/Logger.h"
#include <string>

namespace {

QString fromStdString(std::string const& value)
{
    return QString::fromUtf8(value.c_str());
}

std::string toStdString(QString const& value)
{
    const QByteArray utf8 = value.toUtf8();
    return std::string(utf8.constData(), static_cast<size_t>(utf8.size()));
}

} // namespace

TaskServiceTableModel::TaskServiceTableModel(cppmicroservices::BundleContext bundleContext,
                                             QObject* parent)
    : QAbstractTableModel(parent)
    , m_bundleContext(bundleContext)
{
    // 获取TaskServiceManager服务
    try {
        auto refs = m_bundleContext.GetServiceReferences<service::ITaskServiceManager>();
        if (!refs.empty()) {
            m_taskServiceManager = m_bundleContext.GetService(refs.front());
            attachListener();
        }
    }
    catch (const std::exception& e) {
        common::Logger::error(tr("[任务服务] 获取任务服务管理器失败：%1")
                                  .arg(QString::fromUtf8(e.what()))
                                  .toStdString());
    }
}

TaskServiceTableModel::~TaskServiceTableModel()
{
    detachListener();
}

int TaskServiceTableModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return static_cast<int>(m_controllers.size());
}

int TaskServiceTableModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return ColCount;
}

Qt::ItemFlags TaskServiceTableModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant TaskServiceTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= rowCount()) {
        return {};
    }
    const auto& controller = m_controllers[static_cast<size_t>(index.row())];
    bool running           = controller->status() ==
                   service::ITaskServiceManager::ITaskServiceController::TaskServiceStatus::Running;

    if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
        switch (index.column()) {
            case ColName: return fromStdString(controller->serviceName());
            case ColBundle: return fromStdString(controller->symbolicName());
            case ColStatus: return running ? tr("运行中") : tr("已停止");
            case ColActions: return {};
            default: return {};
        }
    }

    if (role == Qt::UserRole) {
        switch (index.column()) {
            case ColStatus: return running;
            case ColActions: return QVariantList {!running, running, true};
            default: return {};
        }
    }

    return {};
}

QVariant TaskServiceTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QAbstractTableModel::headerData(section, orientation, role);
    }
    switch (section) {
        case ColName: return tr("服务");
        case ColBundle: return tr("Bundle");
        case ColStatus: return tr("状态");
        case ColActions: return tr("操作");
        default: return {};
    }
}

void TaskServiceTableModel::refresh()
{
    if (!m_taskServiceManager) {
        common::Logger::error(tr("[任务服务] 任务服务管理器未初始化").toStdString());

        // 获取TaskServiceManager服务
        try {
            auto refs = m_bundleContext.GetServiceReferences<service::ITaskServiceManager>();
            if (!refs.empty()) {
                m_taskServiceManager = m_bundleContext.GetService(refs.front());
                attachListener();
            }
        }
        catch (const std::exception& e) {
            common::Logger::error(tr("[任务服务] 获取任务服务管理器失败：%1")
                                      .arg(QString::fromUtf8(e.what()))
                                      .toStdString());
        }
    }
    if (!m_taskServiceManager)
        return;

    beginResetModel();
    m_controllers.clear();
    refreshControllers();
    endResetModel();
}

void TaskServiceTableModel::refreshControllers()
{
    if (!m_taskServiceManager) {
        return;
    }

    try {
        auto controllers = m_taskServiceManager->listTaskControllers();
        for (const auto& controller : controllers) {
            // 检查是否已存在
            bool exists = false;
            for (const auto& existingController : m_controllers) {
                if (existingController->symbolicName() == controller->symbolicName() &&
                    existingController->serviceName() == controller->serviceName())
                {
                    exists = true;
                    break;
                }
            }
            if (!exists) {
                // 添加状态回调
                const size_t index = m_controllers.size();
                controller->setStatusCallback(
                    [this,
                     index](service::ITaskServiceManager::ITaskServiceController::TaskServiceStatus
                                status) { onStatusChanged(static_cast<int>(index), status); });
                m_controllers.push_back(controller);
            }
        }
    }
    catch (const std::exception& e) {
        common::Logger::error(
            tr("[任务服务] 刷新控制器列表失败：%1").arg(QString::fromUtf8(e.what())).toStdString());
    }
}

bool TaskServiceTableModel::attachListener()
{
    if (!m_taskServiceManager) {
        return false;
    }

    try {
        m_taskServiceManager->setControllerEventCallback(
            [this](std::shared_ptr<service::ITaskServiceManager::ITaskServiceController> controller,
                   service::ITaskServiceManager::ControllerEvent event) {
                QMetaObject::invokeMethod(
                    this, [this, controller, event]() { onControllerEvent(controller, event); });
            });
        return true;
    }
    catch (const std::exception& e) {
        common::Logger::error(tr("[任务服务] 注册控制器事件回调失败：%1")
                                  .arg(QString::fromUtf8(e.what()))
                                  .toStdString());
        return false;
    }
}

void TaskServiceTableModel::detachListener()
{
    if (!m_taskServiceManager) {
        return;
    }

    try {
        m_taskServiceManager->setControllerEventCallback(nullptr);
    }
    catch (...) {
        // 忽略异常
    }
}

void TaskServiceTableModel::onControllerEvent(
    std::shared_ptr<service::ITaskServiceManager::ITaskServiceController> controller,
    service::ITaskServiceManager::ControllerEvent event)
{
    switch (event) {
        case service::ITaskServiceManager::ControllerEvent::Registered: {
            // 检查是否已存在
            bool exists = false;
            for (const auto& existingController : m_controllers) {
                if (existingController->symbolicName() == controller->symbolicName() &&
                    existingController->serviceName() == controller->serviceName())
                {
                    exists = true;
                    break;
                }
            }
            if (!exists) {
                const int newRow = static_cast<int>(m_controllers.size());
                beginInsertRows(QModelIndex(), newRow, newRow);

                // 添加状态回调
                controller->setStatusCallback(
                    [this,
                     newRow](service::ITaskServiceManager::ITaskServiceController::TaskServiceStatus
                                 status) { onStatusChanged(newRow, status); });

                m_controllers.push_back(controller);
                endInsertRows();

                common::Logger::info(tr("[任务服务] 已注册：%1")
                                         .arg(fromStdString(controller->serviceName()))
                                         .toStdString());
            }
            break;
        }
        case service::ITaskServiceManager::ControllerEvent::Unregistered: {
            // 查找并移除
            int index = -1;
            for (size_t i = 0; i < m_controllers.size(); ++i) {
                if (m_controllers[i]->symbolicName() == controller->symbolicName() &&
                    m_controllers[i]->serviceName() == controller->serviceName())
                {
                    index = static_cast<int>(i);
                    break;
                }
            }
            if (index >= 0) {
                QString name = fromStdString(m_controllers[index]->serviceName());
                beginRemoveRows(QModelIndex(), index, index);
                m_controllers.erase(m_controllers.begin() + index);
                endRemoveRows();

                common::Logger::info(tr("[任务服务] 已注销：%1").arg(name).toStdString());
            }
            break;
        }
    }
}

void TaskServiceTableModel::onStatusChanged(
    int index, service::ITaskServiceManager::ITaskServiceController::TaskServiceStatus status)
{
    if (index >= 0 && index < rowCount()) {
        emit dataChanged(createIndex(index, ColStatus), createIndex(index, ColActions));

        const auto& controller = m_controllers[static_cast<size_t>(index)];
        if (status ==
            service::ITaskServiceManager::ITaskServiceController::TaskServiceStatus::Running)
        {
            common::Logger::info(tr("[任务服务] %1 已启动")
                                     .arg(fromStdString(controller->serviceName()))
                                     .toStdString());
        }
        else {
            common::Logger::info(tr("[任务服务] %1 已停止")
                                     .arg(fromStdString(controller->serviceName()))
                                     .toStdString());
        }
    }
}

bool TaskServiceTableModel::isRunning(int row) const
{
    if (row < 0 || row >= m_controllers.size()) {
        return false;
    }

    const auto& controller = m_controllers[static_cast<size_t>(row)];
    return controller->status() ==
           service::ITaskServiceManager::ITaskServiceController::TaskServiceStatus::Running;
}

bool TaskServiceTableModel::startService(int row)
{
    if (row < 0 || row >= m_controllers.size() || !m_taskServiceManager) {
        return false;
    }

    try {
        auto controller = m_controllers[static_cast<size_t>(row)];
        if (controller->status() ==
            service::ITaskServiceManager::ITaskServiceController::TaskServiceStatus::Running)
        {
            return true; // 已经在运行
        }

        auto config = controller->createConfig();
        return controller->start(config);
    }
    catch (const std::exception& e) {
        common::Logger::error(
            tr("[任务服务] 启动服务失败：%1").arg(QString::fromUtf8(e.what())).toStdString());
        return false;
    }
}

void TaskServiceTableModel::stopService(int row)
{
    if (row < 0 || row >= m_controllers.size() || !m_taskServiceManager) {
        return;
    }

    try {
        auto controller = m_controllers[static_cast<size_t>(row)];
        controller->stop();
    }
    catch (const std::exception& e) {
        common::Logger::error(
            tr("[任务服务] 停止服务失败：%1").arg(QString::fromUtf8(e.what())).toStdString());
    }
}
