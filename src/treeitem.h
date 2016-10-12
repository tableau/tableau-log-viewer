#ifndef TREEITEM_H
#define TREEITEM_H

#include <QList>
#include <QVariant>
#include <QVector>

class TreeItem
{
public:
    explicit TreeItem(const QVector<QVariant> &Data, TreeItem *Parent = 0);
    ~TreeItem();

    TreeItem *Child(int number);
    int ChildCount() const;
    int ColumnCount() const;
    QVariant Data(int column) const;
    TreeItem * AddChild();
    bool InsertChildren(int position, int count, int columns);
    bool InsertColumns(int position, int columns);
    TreeItem *Parent();
    bool RemoveChildren(int position, int count);
    bool RemoveColumns(int position, int columns);
    int ChildNumber() const;
    bool SetData(int column, const QVariant &value);

private:
    QList<TreeItem*> m_childItems;
    QVector<QVariant> m_itemData;
    TreeItem * m_parentItem;
};

#endif // TREEITEM_H
