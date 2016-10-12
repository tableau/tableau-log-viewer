#include "treeitem.h"

#include <QStringList>

TreeItem::TreeItem(const QVector<QVariant> &data, TreeItem *parent)
{
    m_parentItem = parent;
    m_itemData = data;
}

TreeItem::~TreeItem()
{
    qDeleteAll(m_childItems);
}

TreeItem *TreeItem::Child(int number)
{
    return m_childItems.value(number);
}

int TreeItem::ChildCount() const
{
    return m_childItems.count();
}

int TreeItem::ChildNumber() const
{
    if (m_parentItem)
        return m_parentItem->m_childItems.indexOf(const_cast<TreeItem*>(this));

    return 0;
}

int TreeItem::ColumnCount() const
{
    return m_itemData.count();
}

QVariant TreeItem::Data(int column) const
{
    return m_itemData.value(column);
}

bool TreeItem::InsertChildren(int position, int count, int columns)
{
    if (position < 0 || position > m_childItems.size())
        return false;

    for (int row = 0; row < count; ++row)
    {
        QVector<QVariant> data(columns);
        TreeItem *item = new TreeItem(data, this);
        m_childItems.insert(position, item);
    }

    return true;
}

TreeItem * TreeItem::AddChild()
{
    InsertChildren(ChildCount(), 1, ColumnCount());
    return Child(ChildCount() - 1);
}

bool TreeItem::InsertColumns(int position, int columns)
{
    if (position < 0 || position > m_itemData.size())
        return false;

    for (int column = 0; column < columns; ++column)
        m_itemData.insert(position, QVariant());

    foreach (TreeItem *child, m_childItems)
        child->InsertColumns(position, columns);

    return true;
}

TreeItem *TreeItem::Parent()
{
    return m_parentItem;
}

bool TreeItem::RemoveChildren(int position, int count)
{
    if (position < 0 || position + count > m_childItems.size())
        return false;

    for (int row = 0; row < count; ++row)
        delete m_childItems.takeAt(position);

    return true;
}

bool TreeItem::RemoveColumns(int position, int columns)
{
    if (position < 0 || position + columns > m_itemData.size())
        return false;

    for (int column = 0; column < columns; ++column)
        m_itemData.remove(position);

    foreach (TreeItem *child, m_childItems)
        child->RemoveColumns(position, columns);

    return true;
}

bool TreeItem::SetData(int column, const QVariant &value)
{
    if (column < 0 || column >= m_itemData.size())
        return false;

    m_itemData[column] = value;
    return true;
}
