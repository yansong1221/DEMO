#pragma once

#include "PluginBundleTableModel.h"
#include <kddockwidgets/qtwidgets/views/DockWidget.h>
#include <memory>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QPushButton;
class QTableView;
QT_END_NAMESPACE

class PluginBundleActionDelegate;

namespace cppmicroservices {
class Framework;
}

class BundleManagerDockWidget : public KDDockWidgets::QtWidgets::DockWidget
{
    Q_OBJECT

public:
    explicit BundleManagerDockWidget(cppmicroservices::Framework* framework,
                                      QWidget* parent = nullptr);
    ~BundleManagerDockWidget() override;

    QString currentPluginDir() const;
    void setPluginDir(const QString& dir);

    void refreshBundleList();
    void loadBundle(int row);
    void unloadBundle(int row);

signals:
    void logMessage(const QString& message);
    void bundleLoaded(int row);
    void bundleUnloaded(int row);

private slots:
    void onBrowsePluginFolder();
    void onRefreshBundleList();
    void onLoadBundleRow(int row);
    void onUnloadBundleRow(int row);
    void onBundleTableSelectionChanged();

private:
    void setupUI();
    void setupConnections();

    cppmicroservices::Framework* m_framework = nullptr;
    
    // UI elements
    QLineEdit* m_pluginDirEdit = nullptr;
    QPushButton* m_browseDirBtn = nullptr;
    QPushButton* m_refreshListBtn = nullptr;
    QTableView* m_bundleView = nullptr;
    
    // Model and delegate
    PluginBundleTableModel* m_bundleModel = nullptr;
    PluginBundleActionDelegate* m_actionDelegate = nullptr;
};
