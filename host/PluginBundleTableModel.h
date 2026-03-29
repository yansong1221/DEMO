#pragma once

#include <QAbstractTableModel>
#include <cppmicroservices/Bundle.h>
#include <cppmicroservices/BundleEvent.h>
#include <cppmicroservices/ListenerToken.h>
#include <QString>
#include <QStringList>
#include <vector>

namespace cppmicroservices {
    class Framework;
}

struct PluginBundleRow {
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
};

struct HostRowState {
    QString label;
    bool startEnabled = true;
    bool stopEnabled = false;
};

class PluginBundleTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    enum Column {
        ColFile = 0,
        ColSymbolicName,
        ColVersion,
        ColDescription,
        ColVendor,
        ColHostState,
        ColActions,
        ColCount
    };

    explicit PluginBundleTableModel(QObject* parent = nullptr);
    ~PluginBundleTableModel() override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void setHostFramework(cppmicroservices::Framework* framework);
    void setRows(QList<PluginBundleRow> rows);
    QString manifestTextAtRow(int row) const;
    QString absPathAtRow(int row) const;
    QString symbolicNameAtRow(int row) const;

    void refreshAllHostStates();

    bool attachBundleListener();
    void detachBundleListener();

    void rescanPluginDirectory(QString const& root);
    void installBundlesFromFiles(QStringList const& absolutePaths);
    void startBundleRow(int row);
    void stopBundleRow(int row);

signals:
    void bundleLog(QString const& message);

private:
    QString bundleStateLabel(cppmicroservices::Bundle::State state) const;
    HostRowState resolveHostRowState(QString const& absPath, std::string const& sym) const;
    void applyBundleEvent(cppmicroservices::BundleEvent const& evt);
    int findRowForHostBundle(cppmicroservices::Bundle const& b) const;
    void refreshHostStateAtRow(int row);
    void startAllBundlesInHost();
    void ensureServiceTimeStartedIfPresent();

    QList<PluginBundleRow> m_rows;
    cppmicroservices::Framework* m_hostFramework = nullptr;
    cppmicroservices::ListenerToken m_bundleListenerToken;
};
