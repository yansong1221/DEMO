#pragma once

#include <QStyledItemDelegate>

class TaskServiceActionDelegate : public QStyledItemDelegate
{
    Q_OBJECT

  public:
    explicit TaskServiceActionDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter, QStyleOptionViewItem const& option, QModelIndex const& index) const override;
    QSize sizeHint(QStyleOptionViewItem const& option, QModelIndex const& index) const override;
    bool editorEvent(QEvent* event,
                     QAbstractItemModel* model,
                     QStyleOptionViewItem const& option,
                     QModelIndex const& index) override;

  signals:
    void startStopRequested(int row);
    void configRequested(int row);

  private:
    static QRect startStopButtonRect(QRect const& cell);
    static QRect configButtonRect(QRect const& cell);
    static void drawActionButton(QPainter* painter,
                                 QRect const& rect,
                                 QString const& text,
                                 QColor const& bgColor,
                                 QColor const& textColor,
                                 bool enabled);
};
