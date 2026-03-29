#include "PluginBundleActionDelegate.h"

#include <QAbstractItemView>
#include <QMouseEvent>
#include <QPainter>

namespace {

constexpr int kMargin = 4;

} // namespace

PluginBundleActionDelegate::PluginBundleActionDelegate(QObject* parent)
    : QStyledItemDelegate(parent) {
}

QRect PluginBundleActionDelegate::actionButtonRect(const QRect& cell) {
    return cell.adjusted(kMargin, 2, -kMargin, -2);
}

void PluginBundleActionDelegate::drawActionButton(
    QPainter* painter,
    const QRect& rect,
    const QString& text,
    const QColor& bgColor,
    const QColor& textColor,
    bool enabled) {
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    if (enabled) {
        painter->setPen(QColor(150, 150, 150));
        painter->setBrush(bgColor);
        painter->drawRoundedRect(rect.adjusted(0, 0, -1, -1), 3, 3);
        painter->setPen(textColor);
    } else {
        painter->setPen(QColor(200, 200, 200));
        painter->setBrush(QColor(240, 240, 240));
        painter->drawRoundedRect(rect.adjusted(0, 0, -1, -1), 3, 3);
        painter->setPen(QColor(180, 180, 180));
    }

    painter->drawText(rect, Qt::AlignCenter, text);
    painter->restore();
}

void PluginBundleActionDelegate::paint(
    QPainter* painter,
    const QStyleOptionViewItem& option,
    const QModelIndex& index) const {
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
    }
    else {
        painter->fillRect(option.rect, option.palette.base());
    }

    bool isLoaded = false;
    const QVariant v = index.data(Qt::UserRole);
    if (v.canConvert<QVariantList>()) {
        const QVariantList list = v.toList();
        if (list.size() >= 2) {
            // list[0] = loadEnabled, list[1] = unloadEnabled
            // 如果卸载可用，说明已加载
            isLoaded = list[1].toBool();
        }
    }

    const QRect btnRect = actionButtonRect(option.rect);

    if (isLoaded) {
        // 已加载状态：显示红色"卸载"按钮
        drawActionButton(painter, btnRect, QObject::tr("卸载"),
                         QColor(220, 80, 80),   // 红色背景
                         QColor(255, 255, 255), // 白色文字
                         true);
    } else {
        // 未加载状态：显示普通"加载"按钮
        drawActionButton(painter, btnRect, QObject::tr("加载"),
                         QColor(240, 240, 240), // 灰色背景
                         QColor(0, 0, 0),       // 黑色文字
                         true);
    }
}

QSize PluginBundleActionDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(80, 28);
}

bool PluginBundleActionDelegate::editorEvent(
    QEvent* event,
    QAbstractItemModel* model,
    const QStyleOptionViewItem& option,
    const QModelIndex& index) {
    Q_UNUSED(model);
    Q_UNUSED(option);

    if (event->type() != QEvent::MouseButtonRelease) {
        return QStyledItemDelegate::editorEvent(event, model, option, index);
    }

    auto* me = static_cast<QMouseEvent*>(event);
    if (me->button() != Qt::LeftButton) {
        return false;
    }

    auto* view = qobject_cast<QAbstractItemView*>(parent());
    if (!view) {
        return false;
    }

    const QRect cell = view->visualRect(index);
    const QPoint p = me->pos();

    bool loadEnabled = false;
    bool unloadEnabled = false;
    const QVariant v = index.data(Qt::UserRole);
    if (v.canConvert<QVariantList>()) {
        const QVariantList list = v.toList();
        if (list.size() >= 2) {
            loadEnabled = list[0].toBool();
            unloadEnabled = list[1].toBool();
        }
    }

    if (actionButtonRect(cell).contains(p)) {
        if (unloadEnabled) {
            // 当前是已加载状态，发送卸载请求
            emit unloadRequested(index.row());
        } else if (loadEnabled) {
            // 当前是未加载状态，发送加载请求
            emit loadRequested(index.row());
        }
        return true;
    }

    return false;
}
