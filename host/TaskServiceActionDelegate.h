#pragma once

#include <QStyledItemDelegate>

class TaskServiceActionDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit TaskServiceActionDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(
        QEvent* event,
        QAbstractItemModel* model,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) override;

signals:
    void startStopRequested(int row);
    void configRequested(int row);

private:
    static QRect startStopButtonRect(const QRect& cell);
    static QRect configButtonRect(const QRect& cell);
    static void drawActionButton(
        QPainter* painter,
        const QRect& rect,
        const QString& text,
        const QColor& bgColor,
        const QColor& textColor,
        bool enabled);
};
