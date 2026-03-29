#include "BundleManagerDockWidget.h"
#include "PluginBundleActionDelegate.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTableView>
#include <QVBoxLayout>

#include "common/Logger.h"
#include <cppmicroservices/Framework.h>

namespace {

std::string toUtf8StdString(QString const& value)
{
    const QByteArray utf8 = value.toUtf8();
    return std::string(utf8.constData(), static_cast<size_t>(utf8.size()));
}

// 获取默认插件目录（应用程序目录下的 bundles 文件夹）
QString getDefaultPluginDir()
{
    return QCoreApplication::applicationDirPath() + "/bundles";
}

} // namespace

BundleManagerDockWidget::BundleManagerDockWidget(cppmicroservices::Framework* framework,
                                                 QWidget* parent)
    : KDDockWidgets::QtWidgets::DockWidget(QStringLiteral("BundleManager"))
    , m_framework(framework)
{
    Q_UNUSED(parent)
    setupUI();
    setupConnections();

    // 启动监听器
    if (!m_bundleModel->attachBundleListener()) {
        common::Logger::info(tr("[监听] 在模型中注册 Bundle 监听器失败。").toStdString());
    }
}

BundleManagerDockWidget::~BundleManagerDockWidget()
{
    m_bundleModel->detachBundleListener();
}

void BundleManagerDockWidget::setupUI()
{
    auto* centralWidget = new QWidget(this);
    auto* mainLayout    = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);

    // 工具栏
    auto* toolLayout = new QHBoxLayout();
    toolLayout->setSpacing(4);

    m_refreshListBtn = new QPushButton(tr("刷新 Bundles"), this);
    toolLayout->addWidget(m_refreshListBtn);
    toolLayout->addStretch();

    mainLayout->addLayout(toolLayout);

    // Bundle 表格
    m_bundleView = new QTableView(this);
    m_bundleView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_bundleView->setAlternatingRowColors(true);
    m_bundleView->setSelectionMode(QAbstractItemView::NoSelection);
    m_bundleView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_bundleView->verticalHeader()->setVisible(false);
    m_bundleView->verticalHeader()->setDefaultSectionSize(30);

    m_bundleModel = new PluginBundleTableModel(this);
    m_bundleModel->setHostFramework(m_framework);

    m_bundleView->setModel(m_bundleModel);
    m_bundleView->horizontalHeader()->setStretchLastSection(false);
    m_bundleView->horizontalHeader()->setSectionResizeMode(PluginBundleTableModel::ColDescription,
                                                           QHeaderView::Stretch);
    m_bundleView->horizontalHeader()->setSectionResizeMode(PluginBundleTableModel::ColActions,
                                                           QHeaderView::Fixed);
    m_bundleView->setColumnWidth(PluginBundleTableModel::ColActions, 90);

    m_actionDelegate = new PluginBundleActionDelegate(m_bundleView);
    m_bundleView->setItemDelegateForColumn(PluginBundleTableModel::ColActions, m_actionDelegate);

    mainLayout->addWidget(m_bundleView);

    setWidget(centralWidget);
    setTitle(tr("Bundle 管理"));
}

void BundleManagerDockWidget::setupConnections()
{
    connect(m_refreshListBtn,
            &QPushButton::clicked,
            this,
            &BundleManagerDockWidget::onRefreshBundleList);
    connect(m_bundleView->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            &BundleManagerDockWidget::onBundleTableSelectionChanged);
    connect(m_actionDelegate,
            &PluginBundleActionDelegate::loadRequested,
            this,
            &BundleManagerDockWidget::onLoadBundleRow);
    connect(m_actionDelegate,
            &PluginBundleActionDelegate::unloadRequested,
            this,
            &BundleManagerDockWidget::onUnloadBundleRow);
}

void BundleManagerDockWidget::refreshBundleList()
{
    m_bundleModel->rescanPluginDirectory(getDefaultPluginDir());
}

void BundleManagerDockWidget::loadBundle(int row)
{
    m_bundleModel->startBundleRow(row);
    emit bundleLoaded(row);
}

void BundleManagerDockWidget::unloadBundle(int row)
{
    m_bundleModel->stopBundleRow(row);
    emit bundleUnloaded(row);
}

int BundleManagerDockWidget::bundleCount() const
{
    return m_bundleModel->rowCount();
}

void BundleManagerDockWidget::onRefreshBundleList()
{
    refreshBundleList();
}

void BundleManagerDockWidget::onLoadBundleRow(int row)
{
    loadBundle(row);
}

void BundleManagerDockWidget::onUnloadBundleRow(int row)
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
        common::Logger::info(tr("[卸载] 已取消：%1").arg(bundleName).toStdString());
        return;
    }

    unloadBundle(row);
}

void BundleManagerDockWidget::onBundleTableSelectionChanged()
{
    const QModelIndex cur = m_bundleView->selectionModel()->currentIndex();
    if (!cur.isValid()) {
        return;
    }
}
