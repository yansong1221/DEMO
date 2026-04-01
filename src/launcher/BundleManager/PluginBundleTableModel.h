#pragma once

#include <QAbstractTableModel>
#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <cppmicroservices/Bundle.h>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/BundleEvent.h>
#include <cppmicroservices/ListenerToken.h>
#include <vector>

struct PluginBundleRow
{
    QString fileName;
    QString absPath;
    QString symbolicName;
    QString version;
    QString description;
    QString vendor;
    QString manifestText;
    QString hostState;
    bool actionStartEnabled = true;
    bool actionStopEnabled = false;

    cppmicroservices::Bundle bundle;
    QFileInfo fileInfo;
};

struct HostRowState
{
    QString label;
    bool startEnabled = true;
    bool stopEnabled = false;
};

class PluginBundleTableModel : public QAbstractTableModel
{
    Q_OBJECT

  public:
    enum Column
    {
        ColFile = 0,
        ColSymbolicName,
        ColVersion,
        ColDescription,
        ColVendor,
        ColHostState,
        ColActions,
        ColCount
    };

    explicit PluginBundleTableModel(cppmicroservices::BundleContext bundleContext, QObject* parent = nullptr);
    ~PluginBundleTableModel() override;

    int rowCount(QModelIndex const& parent = QModelIndex()) const override;
    int columnCount(QModelIndex const& parent = QModelIndex()) const override;
    Qt::ItemFlags flags(QModelIndex const& index) const override;
    QVariant data(QModelIndex const& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void setRows(QList<PluginBundleRow> rows);
    QString manifestTextAtRow(int row) const;
    QString absPathAtRow(int row) const;
    QString symbolicNameAtRow(int row) const;

    void refreshAllHostStates();

    void rescanPluginDirectory(QStringList const& dirs);
    void installBundlesFromFiles(QStringList const& absolutePaths);
    void startBundleRow(int row);
    void stopBundleRow(int row);

    void stopAllBundles();

  private:
    QString bundleStateLabel(cppmicroservices::Bundle::State state) const;
    HostRowState resolveHostRowState(QString const& absPath, std::string const& sym) const;
    void applyBundleEvent(cppmicroservices::BundleEvent const& evt);
    int findRowForHostBundle(cppmicroservices::Bundle const& b) const;
    void refreshHostStateAtRow(int row);
    void startAllBundlesInHost();
    void ensureServiceTimeStartedIfPresent();

    QList<PluginBundleRow> m_rows;
    cppmicroservices::BundleContext m_hostContext;
    cppmicroservices::ListenerToken m_bundleListenerToken;
};
