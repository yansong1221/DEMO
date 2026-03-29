#pragma once

#include <QStyledItemDelegate>

class PluginBundleActionDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit PluginBundleActionDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(
        QEvent* event,
        QAbstractItemModel* model,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) override;

signals:
    void loadRequested(int row);
    void unloadRequested(int row);

private:
    static QRect actionButtonRect(const QRect& cell);
    static void drawActionButton(
        QPainter* painter,
        const QRect& rect,
        const QString& text,
        const QColor& bgColor,
        const QColor& textColor,
        bool enabled);
};
