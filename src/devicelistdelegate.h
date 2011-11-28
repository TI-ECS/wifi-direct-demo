#ifndef _DEVICELISTDELEGATE_H_
#define _DEVICELISTDELEGATE_H_

#include <QItemDelegate>

class DeviceListDelegate : public QItemDelegate
{
Q_OBJECT

public:
    DeviceListDelegate(QObject *parent = 0);
    ~DeviceListDelegate();

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;
    int spacing() const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index ) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;
    void setSpacing(int space);

private:
    int m_space;
};

#endif /* _DEVICELISTDELEGATE_H_ */
