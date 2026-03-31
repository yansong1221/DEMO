#include "TaskServiceTableModel.h"

#include <QTimer>
#include <cppmicroservices/Bundle.h>
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

TaskServiceEntry::TaskServiceEntry(QObject* parent)
    : QObject(parent)
{
}

TaskServiceEntry::~TaskServiceEntry()
{
    stopService();
}

QString TaskServiceEntry::serviceBundleName() const
{
    try {
        auto bundle = m_ref.GetBundle();
        if (bundle) {
            return fromStdString(bundle.GetSymbolicName());
        }
    }
    catch (...) {
    }
    return {};
}

bool TaskServiceEntry::startService(std::shared_ptr<service::ITaskService::IBasicConfig> cfg)
{
    if (!service) {
        return false;
    }

    if (isRunning()) {
        return true; // 已经在运行
    }
    config = cfg;
    // 调用基类方法启动线程
    return startThread();
}

bool TaskServiceEntry::stopService(uint32_t millisecond)
{
    if (!isRunning()) {
        return true; // 已经停止
    }

    service->requestStop();

    return stopThread(millisecond);
}

bool TaskServiceEntry::onThreadStart()
{
    if (!service) {
        return false;
    }
    try {
        bool ret = service->onThreadStart(config);
        if (ret) {
            emit serviceStarted(this);
        }
        return ret;
    }
    catch (...) {
        return false;
    }
}

bool TaskServiceEntry::onThreadRun()
{
    if (!service) {
        return false;
    }

    try {
        return service->onThreadRun();
    }
    catch (...) {
        return false;
    }
}

void TaskServiceEntry::onThreadEnd()
{
    if (!service) {
        return;
    }

    try {
        service->onThreadEnd();
    }
    catch (...) {
        // 忽略异常
    }
    // 发射信号通知UI状态改变
    emit serviceStopped(this);
}

TaskServiceTableModel::TaskServiceTableModel(cppmicroservices::BundleContext bundleContext,
                                             QObject* parent)
    : QAbstractTableModel(parent)
    , m_bundleContext(bundleContext)
{
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
    return static_cast<int>(m_entries.size());
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
    const TaskServiceEntry& entry = *m_entries[static_cast<size_t>(index.row())];
    bool running                  = entry.isRunning();

    if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
        switch (index.column()) {
            case ColName: return QString::fromStdString(entry.service->name());
            case ColBundle: return entry.serviceBundleName();
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
    beginResetModel();
    m_entries.clear();
    endResetModel();

    try {
        const auto refs = m_bundleContext.GetServiceReferences<service::ITaskService>();
        for (auto const& ref : refs) {
            handleServiceRegistered(ref);
        }
    }
    catch (const std::exception& e) {
        common::Logger::error(
            tr("[任务服务] 刷新服务列表失败：%1").arg(QString::fromUtf8(e.what())).toStdString());
    }
}

bool TaskServiceTableModel::attachListener()
{
    detachListener();
    if (!m_bundleContext) {
        return false;
    }
    try {
        m_listenerToken =
            m_bundleContext.AddServiceListener([this](cppmicroservices::ServiceEvent const& evt) {
                if (!evt) {
                    return;
                }
                QMetaObject::invokeMethod(this, [this, evt]() { onServiceEvent(evt); });
            });
        return static_cast<bool>(m_listenerToken);
    }
    catch (const std::exception& e) {
        common::Logger::error(
            tr("[任务服务] 注册服务监听失败：%1").arg(QString::fromUtf8(e.what())).toStdString());
        return false;
    }
}

void TaskServiceTableModel::detachListener()
{
    if (m_listenerToken && m_bundleContext) {
        try {
            m_bundleContext.RemoveListener(std::move(m_listenerToken));
        }
        catch (...) {
        }
    }
}

void TaskServiceTableModel::onServiceEvent(cppmicroservices::ServiceEvent const& evt)
{
    using namespace cppmicroservices;

    try {
        auto type     = evt.GetType();
        auto eventRef = evt.GetServiceReference();

        switch (type) {
            case cppmicroservices::ServiceEvent::SERVICE_REGISTERED: {
                handleServiceRegistered(eventRef);
            } break;
            case cppmicroservices::ServiceEvent::SERVICE_UNREGISTERING: {
                handleServiceUnregistering(eventRef);
            } break;
            case cppmicroservices::ServiceEvent::SERVICE_MODIFIED:
            case cppmicroservices::ServiceEvent::SERVICE_MODIFIED_ENDMATCH:

            {
                handleServiceModified(eventRef);
            } break;
            default: break;
        }
    }
    catch (const std::exception& e) {
        common::Logger::error(
            tr("[任务服务] 处理服务事件失败：%1").arg(QString::fromUtf8(e.what())).toStdString());
    }
}

void TaskServiceTableModel::handleServiceRegistered(
    cppmicroservices::ServiceReferenceBase const& ref)
{
    try {
        auto service = m_bundleContext.GetService<service::ITaskService>(ref);
        if (!service) {
            return;
        }

        // 检查是否已存在
        int existingIndex = findEntryIndexByServiceReference(ref);
        if (existingIndex >= 0) {
            // 已存在则更新
            handleServiceModified(ref);
            return;
        }

        // 添加新条目
        auto entry     = std::make_unique<TaskServiceEntry>(this);
        entry->service = service;
        entry->m_ref   = ref;

        const int newRow = static_cast<int>(m_entries.size());

        // 连接状态改变信号
        connect(
            entry.get(),
            &TaskServiceEntry::serviceStarted,
            this,
            [this](TaskServiceEntry* entry) {
                int row = indexOfEntryRow(entry);
                if (row >= 0 && row < rowCount()) {
                    emit dataChanged(index(row, ColStatus), index(row, ColActions));
                    common::Logger::info(
                        tr("[任务服务] %1 线程已启动")
                            .arg(QString::fromStdString(m_entries.back()->service->name()))
                            .toStdString());
                }
            },
            Qt::QueuedConnection);

        connect(
            entry.get(),
            &TaskServiceEntry::serviceStopped,
            this,
            [this](TaskServiceEntry* entry) {
                int row = indexOfEntryRow(entry);
                if (row >= 0 && row < rowCount()) {
                    emit dataChanged(index(row, ColStatus), index(row, ColActions));
                    common::Logger::info(
                        tr("[任务服务] %1 线程已停止")
                            .arg(QString::fromStdString(m_entries.back()->service->name()))
                            .toStdString());
                }
            },
            Qt::QueuedConnection);

        beginInsertRows(QModelIndex(), newRow, newRow);
        m_entries.push_back(std::move(entry));
        endInsertRows();

        common::Logger::info(tr("[任务服务] 已注册：%1")
                                 .arg(QString::fromStdString(m_entries.back()->service->name()))
                                 .toStdString());
    }
    catch (const std::exception& e) {
        common::Logger::error(
            tr("[任务服务] 处理注册事件失败：%1").arg(QString::fromUtf8(e.what())).toStdString());
    }
}

void TaskServiceTableModel::handleServiceUnregistering(
    cppmicroservices::ServiceReferenceBase const& ref)
{
    int index = -1;
    for (int i = 0; i < m_entries.size(); ++i) {
        const cppmicroservices::ServiceReferenceBase currentRef = m_entries[i]->m_ref;
        if (currentRef == ref) {
            index = i;
            break;
        }
    }
    if (index < 0) {
        return;
    }

    QString name = QString::fromStdString(m_entries[index]->service->name());
    beginRemoveRows(QModelIndex(), index, index);
    m_entries.erase(m_entries.begin() + index);
    endRemoveRows();

    common::Logger::info(tr("[任务服务] 已注销：%1").arg(name).toStdString());
}

void TaskServiceTableModel::handleServiceModified(cppmicroservices::ServiceReferenceBase const& ref)
{
    int index = findEntryIndexByServiceReference(ref);
    if (index < 0) {
        // 如果不存在，尝试作为新服务添加
        handleServiceRegistered(ref);
        return;
    }

    try {
        auto service = m_bundleContext.GetService<service::ITaskService>(ref);
        if (!service) {
            return;
        }

        TaskServiceEntry& entry = *m_entries[index];
        entry.service           = service;

        emit dataChanged(createIndex(index, 0), createIndex(index, ColCount - 1));
    }
    catch (const std::exception& e) {
        common::Logger::error(
            tr("[任务服务] 处理修改事件失败：%1").arg(QString::fromUtf8(e.what())).toStdString());
    }
}

int TaskServiceTableModel::findEntryIndexByServiceReference(
    cppmicroservices::ServiceReferenceBase const& ref) const
{
    for (size_t i = 0; i < m_entries.size(); ++i) {
        if (m_entries[i]->m_ref == ref) {
            return i;
        }
    }
    return -1;
}


TaskServiceEntry* TaskServiceTableModel::entryAt(int row)
{
    if (row < 0 || row >= m_entries.size()) {
        return nullptr;
    }
    return m_entries[row].get();
}

const TaskServiceEntry* TaskServiceTableModel::entryAt(int row) const
{
    if (row < 0 || row >= m_entries.size()) {
        return nullptr;
    }
    return m_entries[row].get();
}

int TaskServiceTableModel::indexOfEntryRow(const TaskServiceEntry* entry) const
{
    for (int i = 0; i < rowCount(); ++i) {
        if (entryAt(i) == entry) {
            return i;
        }
    }
    return -1;
}

bool TaskServiceTableModel::isRunning(int row) const
{
    const auto* entry = entryAt(row);
    return entry ? entry->isRunning() : false;
}