#pragma once

#include <QStyledItemDelegate>

class PluginBundleActionDelegate : public QStyledItemDelegate
{
    Q_OBJECT

  public:
    explicit PluginBundleActionDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter, QStyleOptionViewItem const& option, QModelIndex const& index) const override;
    QSize sizeHint(QStyleOptionViewItem const& option, QModelIndex const& index) const override;
    bool editorEvent(QEvent* event,
                     QAbstractItemModel* model,
                     QStyleOptionViewItem const& option,
                     QModelIndex const& index) override;

  signals:
    void loadRequested(int row);
    void unloadRequested(int row);

  private:
    static QRect actionButtonRect(QRect const& cell);
    static void drawActionButton(QPainter* painter,
                                 QRect const& rect,
                                 QString const& text,
                                 QColor const& bgColor,
                                 QColor const& textColor,
                                 bool enabled);
};
