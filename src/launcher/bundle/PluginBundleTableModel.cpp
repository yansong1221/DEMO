#include "PluginBundleTableModel.h"

#include <QDir>
#include <QFileInfo>
#include <QTimer>
#include <QUuid>

#include <cppmicroservices/AnyMap.h>
#include <cppmicroservices/Bundle.h>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/Constants.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/FrameworkFactory.h>

#include "common/Logger.h"
#include <algorithm>
#include <chrono>
#include <string>

using namespace cppmicroservices;

namespace {

constexpr char kServiceTimeSymbolic[] = "service_time_systemclock";

std::string pathForInstall(QString const& qPath)
{
    const QByteArray utf8 = QFileInfo(qPath).absoluteFilePath().toUtf8();
    return std::string(utf8.constData(), static_cast<size_t>(utf8.size()));
}

bool sameBundleLocation(QString const& absPath, std::string const& bundleLocation)
{
    if (bundleLocation.empty()) {
        return false;
    }
    const QString a = QFileInfo(absPath).absoluteFilePath();
    const QString b = QFileInfo(QString::fromUtf8(bundleLocation.c_str())).absoluteFilePath();
    return a.compare(b, Qt::CaseInsensitive) == 0;
}

Bundle findHostBundle(BundleContext ctx, QString const& absPath, std::string const& symbolicName)
{
    if (!ctx) {
        return Bundle {};
    }
    try {
        for (auto& b : ctx.GetBundles()) {
            if (b.GetBundleId() == 0) {
                continue;
            }
            if (b.GetSymbolicName() != symbolicName) {
                continue;
            }
            if (!sameBundleLocation(absPath, b.GetLocation())) {
                continue;
            }
            return b;
        }
    }
    catch (...) {
    }
    return Bundle {};
}

QString anyMapToText(AnyMap const& headers)
{
    QString lines;
    for (auto const& entry : headers) {
        QString key = QString::fromUtf8(entry.first.c_str());
        QString val = QString::fromUtf8(entry.second.ToString().c_str());
        lines += key + QLatin1String(": ") + val + QLatin1Char('\n');
    }
    return lines;
}

QString headerString(AnyMap const& headers, std::string const& key)
{
    auto it = headers.find(key);
    if (it == headers.end()) {
        return {};
    }
    return QString::fromUtf8(it->second.ToString().c_str());
}

bool bundleEventAffectsHostStateColumn(BundleEvent::Type t)
{
    switch (t) {
        case BundleEvent::BUNDLE_INSTALLED:
        case BundleEvent::BUNDLE_RESOLVED:
        case BundleEvent::BUNDLE_UNRESOLVED:
        case BundleEvent::BUNDLE_STARTING:
        case BundleEvent::BUNDLE_STARTED:
        case BundleEvent::BUNDLE_STOPPING:
        case BundleEvent::BUNDLE_STOPPED:
        case BundleEvent::BUNDLE_UNINSTALLED:
        case BundleEvent::BUNDLE_UPDATED:
        case BundleEvent::BUNDLE_LAZY_ACTIVATION: return true;
        default: return false;
    }
}

} // namespace

PluginBundleTableModel::PluginBundleTableModel(cppmicroservices::BundleContext bundleContext,
                                               QObject* parent)
    : QAbstractTableModel(parent)
    , m_hostContext(bundleContext)
{
    m_bundleListenerToken = m_hostContext.AddBundleListener([this](BundleEvent const& evt) {
        if (!evt) {
            return;
        }
        const BundleEvent ev = evt;
        QMetaObject::invokeMethod(this, [this, ev]() { applyBundleEvent(ev); });
    });
}

PluginBundleTableModel::~PluginBundleTableModel()
{
    if (m_bundleListenerToken && m_hostContext) {
        try {
            m_hostContext.RemoveListener(std::move(m_bundleListenerToken));
        }
        catch (...) {
        }
    }
}

QString PluginBundleTableModel::bundleStateLabel(Bundle::State state) const
{
    if ((state & Bundle::STATE_ACTIVE) != 0) {
        return tr("运行中");
    }
    if ((state & Bundle::STATE_STARTING) != 0) {
        return tr("正在启动");
    }
    if ((state & Bundle::STATE_STOPPING) != 0) {
        return tr("正在停止");
    }
    if ((state & Bundle::STATE_RESOLVED) != 0) {
        return tr("已解析");
    }
    if ((state & Bundle::STATE_INSTALLED) != 0) {
        return tr("已安装");
    }
    if ((state & Bundle::STATE_UNINSTALLED) != 0) {
        return tr("已卸载");
    }
    return tr("未知");
}

HostRowState PluginBundleTableModel::resolveHostRowState(QString const& absPath,
                                                         std::string const& sym) const
{
    Bundle b = findHostBundle(m_hostContext, absPath, sym);
    if (!b) {
        return {tr("未安装"), true, false};
    }
    try {
        const auto st        = b.GetState();
        const bool active    = (st & Bundle::STATE_ACTIVE) != 0;
        const bool transient = (st & (Bundle::STATE_STARTING | Bundle::STATE_STOPPING)) != 0;
        return {bundleStateLabel(st), !active && !transient, active};
    }
    catch (...) {
        return {tr("?"), false, false};
    }
}

void PluginBundleTableModel::applyBundleEvent(BundleEvent const& evt)
{
    if (!m_hostContext || !evt) {
        return;
    }
    const BundleEvent::Type t = evt.GetType();
    if (!bundleEventAffectsHostStateColumn(t)) {
        return;
    }
    Bundle b = evt.GetBundle();
    if (!b || b.GetBundleId() == 0) {
        return;
    }
    refreshHostStateAtRow(findRowForHostBundle(b));
}

int PluginBundleTableModel::findRowForHostBundle(Bundle const& b) const
{
    if (!b || b.GetBundleId() == 0) {
        return -1;
    }
    try {
        const std::string loc = b.GetLocation();
        const std::string sym = b.GetSymbolicName();
        for (int r = 0; r < static_cast<int>(m_rows.size()); ++r) {
            if (!sameBundleLocation(m_rows[static_cast<size_t>(r)].absPath, loc)) {
                continue;
            }
            const QByteArray u = m_rows[static_cast<size_t>(r)].symbolicName.toUtf8();
            const std::string rowSym(u.constData(), static_cast<size_t>(u.size()));
            if (rowSym != sym) {
                continue;
            }
            return r;
        }
    }
    catch (...) {
    }
    return -1;
}

void PluginBundleTableModel::refreshHostStateAtRow(int row)
{
    if (!m_hostContext || row < 0 || row >= static_cast<int>(m_rows.size())) {
        return;
    }
    const QByteArray symUtf8 = m_rows[static_cast<size_t>(row)].symbolicName.toUtf8();
    const std::string sym(symUtf8.constData(), static_cast<size_t>(symUtf8.size()));
    const HostRowState next = resolveHostRowState(m_rows[static_cast<size_t>(row)].absPath, sym);
    PluginBundleRow& rowRef = m_rows[static_cast<size_t>(row)];
    const bool changed      = rowRef.hostState != next.label ||
                         rowRef.actionStartEnabled != next.startEnabled ||
                         rowRef.actionStopEnabled != next.stopEnabled;
    if (!changed) {
        return;
    }
    rowRef.hostState          = next.label;
    rowRef.actionStartEnabled = next.startEnabled;
    rowRef.actionStopEnabled  = next.stopEnabled;
    emit dataChanged(index(row, ColFile), index(row, ColActions), {Qt::DisplayRole, Qt::UserRole});
}

void PluginBundleTableModel::ensureServiceTimeStartedIfPresent()
{
    if (!m_hostContext) {
        return;
    }
    try {
        for (auto& b : m_hostContext.GetBundles()) {
            if (b.GetSymbolicName() != kServiceTimeSymbolic) {
                continue;
            }
            if ((b.GetState() & Bundle::STATE_ACTIVE) != 0) {
                return;
            }
            b.Start();
            return;
        }
    }
    catch (...) {
    }
}

void PluginBundleTableModel::startAllBundlesInHost()
{
    if (!m_hostContext) {
        return;
    }

    try {
        auto bundles = m_hostContext.GetBundles();
        auto iter    = std::find_if(bundles.begin(), bundles.end(), [](Bundle& b) {
            return b.GetSymbolicName() == kServiceTimeSymbolic;
        });
        if (iter != bundles.end()) {
            iter->Start();
        }
        for (auto& bundle : bundles) {
            bundle.Start();
        }
    }
    catch (const std::exception& e) {
        common::Logger::error(QString::fromUtf8("[错误] 启动 Bundle：%1")
                                  .arg(QString::fromUtf8(e.what()))
                                  .toStdString());
    }
}

void PluginBundleTableModel::refreshAllHostStates()
{
    for (int r = 0; r < static_cast<int>(m_rows.size()); ++r) {
        const QByteArray symUtf8 = m_rows[static_cast<size_t>(r)].symbolicName.toUtf8();
        const std::string sym(symUtf8.constData(), static_cast<size_t>(symUtf8.size()));
        const HostRowState next = resolveHostRowState(m_rows[static_cast<size_t>(r)].absPath, sym);
        PluginBundleRow& rowRef = m_rows[static_cast<size_t>(r)];
        if (rowRef.hostState == next.label && rowRef.actionStartEnabled == next.startEnabled &&
            rowRef.actionStopEnabled == next.stopEnabled)
        {
            continue;
        }
        rowRef.hostState          = next.label;
        rowRef.actionStartEnabled = next.startEnabled;
        rowRef.actionStopEnabled  = next.stopEnabled;
        emit dataChanged(
            index(r, ColHostState), index(r, ColActions), {Qt::DisplayRole, Qt::UserRole});
    }
}

void PluginBundleTableModel::rescanPluginDirectory(QStringList const& dirs)
{
#ifdef _WIN32
    const QStringList filters {QStringLiteral("*.dll")};
#else
    const QStringList filters {QStringLiteral("*.so")};
#endif
    QFileInfoList files;

    for (const auto& _dir : dirs) {
        const QString trimmed = _dir.trimmed();
        if (trimmed.isEmpty() || !QDir(trimmed).exists()) {
            common::Logger::error(tr("[列表] 插件目录无效。").toStdString());
            continue;
        }
        QDir dir(trimmed);
        const QFileInfoList found =
            dir.entryInfoList(filters, QDir::Files | QDir::Readable, QDir::Name);
        files.append(found);
    }
    if (files.isEmpty()) {
        setRows({});
        common::Logger::error(tr("[列表] 目录中未找到插件文件。").toStdString());
        return;
    }

    const QString probeId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    const QString storagePath =
        QDir::temp().absoluteFilePath(QStringLiteral("cs_host_probe/") + probeId);
    if (!QDir().mkpath(storagePath)) {
        common::Logger::error(tr("[列表] 无法创建临时目录。").toStdString());
        return;
    }

    FrameworkConfiguration cfg;
    cfg[Constants::FRAMEWORK_STORAGE] = pathForInstall(storagePath);
    cfg[Constants::FRAMEWORK_STORAGE_CLEAN] =
        std::string(Constants::FRAMEWORK_STORAGE_CLEAN_ONFIRSTINIT);

    QList<PluginBundleRow> built;

    try {
        FrameworkFactory factory;
        Framework probeFw = factory.NewFramework(cfg);
        probeFw.Init();
        probeFw.Start();
        auto ctx = probeFw.GetBundleContext();

        for (const QFileInfo& fi : files) {
            const QString absPath = fi.absoluteFilePath();
            try {
                const std::string loc         = pathForInstall(absPath);
                std::vector<Bundle> installed = ctx.InstallBundles(loc);
                for (Bundle& b : installed) {
                    AnyMap const& headers = b.GetHeaders();
                    PluginBundleRow row;
                    row.fileName     = fi.fileName();
                    row.absPath      = absPath;
                    row.symbolicName = QString::fromUtf8(b.GetSymbolicName().c_str());
                    row.version      = headerString(headers, Constants::BUNDLE_VERSION);
                    row.description  = headerString(headers, Constants::BUNDLE_DESCRIPTION);
                    row.vendor       = headerString(headers, Constants::BUNDLE_VENDOR);
                    row.manifestText = anyMapToText(headers);
                    row.hostState.clear();
                    built.push_back(std::move(row));
                }
            }
            catch (const std::exception& e) {
                common::Logger::error(QString::fromUtf8("[列表] 跳过 %1 — %2")
                                          .arg(absPath, QString::fromUtf8(e.what()))
                                          .toStdString());
            }
        }

        probeFw.Stop();
        probeFw.WaitForStop(std::chrono::milliseconds::max());
    }
    catch (const std::exception& e) {
        common::Logger::error(QString::fromUtf8("[列表] 扫描失败：%1")
                                  .arg(QString::fromUtf8(e.what()))
                                  .toStdString());
    }

    QDir(storagePath).removeRecursively();

    setRows(std::move(built));
    refreshAllHostStates();
    common::Logger::info(
        tr("[列表] 共 %1 条 bundle 记录（来自目录扫描）。").arg(rowCount()).toStdString());
}

void PluginBundleTableModel::installBundlesFromFiles(QStringList const& absolutePaths)
{
    if (!m_hostContext) {
        common::Logger::error(tr("[加载] 框架未就绪。").toStdString());
        return;
    }

    for (const QString& f : absolutePaths) {
        try {
            const std::string loc = pathForInstall(f);
            auto installed        = m_hostContext.InstallBundles(loc);
            common::Logger::info(tr("已安装：%1（%2 个 bundle）")
                                     .arg(f)
                                     .arg(static_cast<int>(installed.size()))
                                     .toStdString());
        }
        catch (const std::exception& e) {
            common::Logger::error(QString::fromUtf8("[失败] %1 — %2")
                                      .arg(f, QString::fromUtf8(e.what()))
                                      .toStdString());
        }
    }
    startAllBundlesInHost();
    refreshAllHostStates();
    common::Logger::info(tr("已尝试按依赖顺序启动全部 bundle。").toStdString());
}

void PluginBundleTableModel::startBundleRow(int row)
{
    if (!m_hostContext) {
        common::Logger::error(tr("[启动] 框架未就绪。").toStdString());
        return;
    }
    if (row < 0 || row >= rowCount()) {
        common::Logger::error(tr("[启动] 无效行。").toStdString());
        return;
    }
    const QString absPath    = absPathAtRow(row);
    const QString symQ       = symbolicNameAtRow(row);
    const QByteArray symUtf8 = symQ.toUtf8();
    const std::string sym(symUtf8.constData(), static_cast<size_t>(symUtf8.size()));
    if (sym.empty()) {
        common::Logger::error(tr("[启动] 缺少 symbolic name，请先刷新列表。").toStdString());
        return;
    }
    try {
        m_hostContext.InstallBundles(pathForInstall(absPath));
        Bundle b = findHostBundle(m_hostContext, absPath, sym);
        if (!b) {
            common::Logger::error(
                tr("[启动] 已安装但仍未匹配到 Bundle（检查路径与 symbolic name）。").toStdString());
            refreshAllHostStates();
            return;
        }
        if (sym != kServiceTimeSymbolic) {
            ensureServiceTimeStartedIfPresent();
        }
        auto s = b.GetSymbolicName();
        b.Start();
        common::Logger::info(
            tr("[启动] 已请求启动：%1").arg(QString::fromUtf8(sym.c_str())).toStdString());
    }
    catch (const std::exception& e) {
        common::Logger::error(
            QString::fromUtf8("[启动] 失败：%1").arg(QString::fromUtf8(e.what())).toStdString());
    }
    refreshAllHostStates();
}

void PluginBundleTableModel::stopBundleRow(int row)
{
    if (!m_hostContext) {
        common::Logger::error(tr("[启动] 框架未就绪。").toStdString());
        return;
    }
    if (row < 0 || row >= rowCount()) {
        common::Logger::error(tr("[启动] 无效行。").toStdString());
        return;
    }

    const QString absPath    = absPathAtRow(row);
    const QString symQ       = symbolicNameAtRow(row);
    const QByteArray symUtf8 = symQ.toUtf8();
    const std::string sym(symUtf8.constData(), static_cast<size_t>(symUtf8.size()));
    if (sym.empty()) {
        common::Logger::error(tr("[启动] 缺少 symbolic name，请先刷新列表。").toStdString());
        return;
    }
    try {
        Bundle b = findHostBundle(m_hostContext, absPath, sym);
        if (!b) {
            common::Logger::error(tr("[停止] 主框架中未安装该 Bundle。").toStdString());
            return;
        }
        b.Stop();
        b.Uninstall();
        common::Logger::info(
            tr("[停止] 已请求停止：%1").arg(QString::fromUtf8(sym.c_str())).toStdString());
    }
    catch (const std::exception& e) {
        common::Logger::error(
            QString::fromUtf8("[停止] 失败：%1").arg(QString::fromUtf8(e.what())).toStdString());
    }
    refreshAllHostStates();
}

void PluginBundleTableModel::stopAllBundles()
{
    return;
    for (int i = 0; i < rowCount(); ++i) {
        stopBundleRow(i);
    }
}

int PluginBundleTableModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return static_cast<int>(m_rows.size());
}

int PluginBundleTableModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return ColCount;
}

Qt::ItemFlags PluginBundleTableModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    if (index.column() == ColActions) {
        return Qt::ItemIsEnabled;
    }
    return QAbstractTableModel::flags(index);
}

QVariant PluginBundleTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= rowCount()) {
        return {};
    }
    const PluginBundleRow& row = m_rows[static_cast<size_t>(index.row())];
    if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
        switch (index.column()) {
            case ColFile: return role == Qt::ToolTipRole ? row.absPath : row.fileName;
            case ColSymbolicName:
                return role == Qt::ToolTipRole ? row.manifestText : row.symbolicName;
            case ColVersion: return row.version;
            case ColDescription: return row.description;
            case ColVendor: return row.vendor;
            case ColHostState: return row.hostState;
            case ColActions: return {};
            default: return {};
        }
    }
    if (index.column() == ColActions && role == Qt::UserRole) {
        return QVariantList {row.actionStartEnabled, row.actionStopEnabled};
    }
    if (role == Qt::AccessibleTextRole && index.column() == ColActions) {
        return tr("启动 停止");
    }
    return {};
}

QVariant PluginBundleTableModel::headerData(int section,
                                            Qt::Orientation orientation,
                                            int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QAbstractTableModel::headerData(section, orientation, role);
    }
    switch (section) {
        case ColFile: return tr("插件文件");
        case ColSymbolicName: return tr("bundle.symbolic_name");
        case ColVersion: return tr("bundle.version");
        case ColDescription: return tr("bundle.description");
        case ColVendor: return tr("bundle.vendor");
        case ColHostState: return tr("主框架状态");
        case ColActions: return tr("操作");
        default: return {};
    }
}

void PluginBundleTableModel::setRows(QList<PluginBundleRow> rows)
{
    beginResetModel();
    m_rows = std::move(rows);
    endResetModel();
}

QString PluginBundleTableModel::manifestTextAtRow(int row) const
{
    if (row < 0 || row >= static_cast<int>(m_rows.size())) {
        return {};
    }
    return m_rows[row].manifestText;
}

QString PluginBundleTableModel::absPathAtRow(int row) const
{
    if (row < 0 || row >= static_cast<int>(m_rows.size())) {
        return {};
    }
    return m_rows[row].absPath;
}

QString PluginBundleTableModel::symbolicNameAtRow(int row) const
{
    if (row < 0 || row >= static_cast<int>(m_rows.size())) {
        return {};
    }
    return m_rows[row].symbolicName;
}
