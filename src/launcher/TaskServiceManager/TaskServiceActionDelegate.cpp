#include "TaskServiceActionDelegate.h"

#include <QAbstractItemView>
#include <QMouseEvent>
#include <QPainter>

namespace
{

    constexpr int kMargin = 4;
    constexpr int kGap = 4;
    constexpr int kButtonWidth = 50;

} // namespace

TaskServiceActionDelegate::TaskServiceActionDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

QRect
TaskServiceActionDelegate::startStopButtonRect(QRect const& cell)
{
    QRect const inner = cell.adjusted(kMargin, 2, -kMargin, -2);
    return QRect(inner.left(), inner.top(), kButtonWidth, inner.height());
}

QRect
TaskServiceActionDelegate::configButtonRect(QRect const& cell)
{
    QRect const inner = cell.adjusted(kMargin, 2, -kMargin, -2);
    return QRect(inner.left() + kButtonWidth + kGap, inner.top(), kButtonWidth, inner.height());
}

void
TaskServiceActionDelegate::drawActionButton(QPainter* painter,
                                            QRect const& rect,
                                            QString const& text,
                                            QColor const& bgColor,
                                            QColor const& textColor,
                                            bool enabled)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    if (enabled)
    {
        painter->setPen(QColor(150, 150, 150));
        painter->setBrush(bgColor);
        painter->drawRoundedRect(rect.adjusted(0, 0, -1, -1), 3, 3);
        painter->setPen(textColor);
    }
    else
    {
        painter->setPen(QColor(200, 200, 200));
        painter->setBrush(QColor(240, 240, 240));
        painter->drawRoundedRect(rect.adjusted(0, 0, -1, -1), 3, 3);
        painter->setPen(QColor(180, 180, 180));
    }

    painter->drawText(rect, Qt::AlignCenter, text);
    painter->restore();
}

void
TaskServiceActionDelegate::paint(QPainter* painter, QStyleOptionViewItem const& option, QModelIndex const& index) const
{
    if (option.state & QStyle::State_Selected)
    {
        painter->fillRect(option.rect, option.palette.highlight());
    }
    else
    {
        painter->fillRect(option.rect, option.palette.base());
    }

    bool isRunning = false;
    bool configEnabled = false;

    QVariant const v = index.data(Qt::UserRole);
    if (v.canConvert<QVariantList>())
    {
        QVariantList const list = v.toList();
        if (list.size() >= 2)
        {
            // list[0] = startEnabled (!isRunning), list[1] = stopEnabled (isRunning)
            isRunning = list[1].toBool();
            configEnabled = list[2].toBool();
        }
    }

    // 开始/停止合并按钮
    if (isRunning)
    {
        // 运行中：显示红色"停止"按钮
        drawActionButton(painter,
                         startStopButtonRect(option.rect),
                         QObject::tr("停止"),
                         QColor(220, 80, 80),   // 红色背景
                         QColor(255, 255, 255), // 白色文字
                         true);
    }
    else
    {
        // 已停止：显示普通"开始"按钮
        drawActionButton(painter,
                         startStopButtonRect(option.rect),
                         QObject::tr("开始"),
                         QColor(240, 240, 240), // 灰色背景
                         QColor(0, 0, 0),       // 黑色文字
                         true);
    }

    // 配置按钮
    drawActionButton(painter,
                     configButtonRect(option.rect),
                     QObject::tr("配置"),
                     QColor(240, 240, 240), // 灰色背景
                     QColor(0, 0, 0),       // 黑色文字
                     configEnabled);
}

QSize
TaskServiceActionDelegate::sizeHint(QStyleOptionViewItem const& option, QModelIndex const& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(kButtonWidth * 2 + kGap + kMargin * 2, 28);
}

bool
TaskServiceActionDelegate::editorEvent(QEvent* event,
                                       QAbstractItemModel* model,
                                       QStyleOptionViewItem const& option,
                                       QModelIndex const& index)
{
    Q_UNUSED(model);
    Q_UNUSED(option);

    if (event->type() != QEvent::MouseButtonRelease)
    {
        return QStyledItemDelegate::editorEvent(event, model, option, index);
    }

    auto* me = static_cast<QMouseEvent*>(event);
    if (me->button() != Qt::LeftButton)
    {
        return false;
    }

    auto* view = qobject_cast<QAbstractItemView*>(parent());
    if (!view)
    {
        return false;
    }

    QRect const cell = view->visualRect(index);
    QPoint const p = me->pos();

    bool startEnabled = false;
    bool stopEnabled = false;
    bool configEnabled = false;

    QVariant const v = index.data(Qt::UserRole);
    if (v.canConvert<QVariantList>())
    {
        QVariantList const list = v.toList();
        if (list.size() >= 3)
        {
            startEnabled = list[0].toBool();
            stopEnabled = list[1].toBool();
            configEnabled = list[2].toBool();
        }
    }

    if (startStopButtonRect(cell).contains(p))
    {
        // 根据当前状态决定是开始还是停止
        if (stopEnabled || startEnabled)
        {
            emit startStopRequested(index.row());
        }
        return true;
    }

    if (configButtonRect(cell).contains(p))
    {
        if (configEnabled)
        {
            emit configRequested(index.row());
        }
        return true;
    }

    return false;
}
