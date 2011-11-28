#include "devicelistdelegate.h"

#include "device.h"

#include <QPainter>
#include <QPoint>
#include <QRect>


DeviceListDelegate::DeviceListDelegate(QObject *parent)
    :QItemDelegate(parent)
{
    m_space = 100;
}

DeviceListDelegate::~DeviceListDelegate()
{
}

QWidget* DeviceListDelegate::createEditor(QWidget *,
                                           const QStyleOptionViewItem &,
                                           const QModelIndex &) const
{
    return NULL; // read only
}

void DeviceListDelegate::setEditorData(QWidget *, const QModelIndex &) const
{
    // read only
}

void DeviceListDelegate::paint(QPainter *painter,
                                const QStyleOptionViewItem &option,
                                const QModelIndex & index ) const
{
    QString name = index.model()->data(index,
                                       Qt::UserRole + 2).toString();
    QString address = index.model()->data(index,
                                          Qt::UserRole + 1).toString();

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->drawRect(option.rect);
    painter->fillRect(option.rect, QColor(40,40,40));

    painter->setPen(Qt::white);
    painter->setFont(QFont("Arial", 20, QFont::Bold));
    if (name.isEmpty()) {
        painter->drawText(option.rect, Qt::AlignCenter, address);
    } else {
        painter->drawText(option.rect, Qt::AlignCenter, name);
        painter->setPen(Qt::darkGray);
        painter->setFont(QFont("Arial", 12));
        QRect aux = option.rect;
        aux.setHeight(aux.height() - 5);
        painter->drawText(aux,
                          Qt::AlignHCenter | Qt::AlignBottom, address);
    }


    painter->restore();
}

void DeviceListDelegate::setModelData(QWidget *, QAbstractItemModel *,
                                       const QModelIndex &) const
{
    // read only
}

QSize DeviceListDelegate::sizeHint(const QStyleOptionViewItem &option,
                                    const QModelIndex &) const
{
    return QSize(option.rect.size().width(), 75);
}

int DeviceListDelegate::spacing() const
{
    return m_space;
}

void DeviceListDelegate::setSpacing(int space)
{
    m_space = space;
}
