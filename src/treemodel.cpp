#include "treemodel.h"

#include "options.h"
#include "treeitem.h"

#include <QJsonObject>
#include <QtWidgets>

TreeModel::TreeModel(const QStringList &headers, const EventListPtr events, QObject *parent)
    : QAbstractItemModel(parent)
{
    QVector<QVariant> rootData;
    foreach (QString header, headers)
        rootData << header;

    m_rootItem = new TreeItem(rootData);
    m_allEvents = events;
    SetupModelData(m_rootItem);

    HighlightOptions defaultHighlightOpts = Options::GetInstance().getDefaultHighlightOpts();
    if (!defaultHighlightOpts.isEmpty())
        m_highlightOpts = defaultHighlightOpts;

    m_colorLibrary = new ColorLibrary();
    m_highlightOnlyMode = false;
    m_liveMode = false;
}

TreeModel::~TreeModel()
{
    delete m_rootItem;
    delete m_colorLibrary;
}

int TreeModel::columnCount(const QModelIndex & /* parent */) const
{
    return m_rootItem->ColumnCount();
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    int col = index.column();
    TreeItem *item = GetItem(index);

    switch (role)
    {
        case Qt::TextColorRole:
        {
            if (col == COL::ID)
                return QColor(Qt::gray);
            break;
        }
        case Qt::BackgroundRole:
        {
            QColor highlightColor(ItemHighlightColor(item));
            if (highlightColor != Qt::white)
            {
                return QBrush(highlightColor);
            }
            break;
        }
        case Qt::UserRole:
        {
            if (col == COL::Time)
            {
                QDateTime dateTime = item->Data(col).toDateTime();
                return dateTime.toMSecsSinceEpoch();
            }
            return item->Data(col);
            break;
        }
        case Qt::DisplayRole:
        {
            if (col == COL::Time)
            {
                QDateTime dateTime = item->Data(col).toDateTime();
                if (!dateTime.isValid())
                    return "";

                if (m_deltaBase == 0)
                    return dateTime.toString("MM/dd/yyyy - hh:mm:ss.zzz");
                else
                    return GetDeltaMSecs(dateTime);
            }
            if (item->Data(col).type() == QVariant::Double)
            {
                return QString::number(item->Data(col).toDouble(), 'f', 3);
            }
            return item->Data(col);
            break;
        }
    }
    return QVariant();
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
}

TreeItem *TreeModel::GetItem(const QModelIndex &index) const
{
    if (index.isValid())
    {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        if (item)
            return item;
    }
    return m_rootItem;
}

TreeItem* TreeModel::GetFirstChildWithKey(const QModelIndex &index, QString key) const
{
    TreeItem *parent = GetItem(index);
    for (int i = 0; i < parent->ChildCount(); i++)
    {
        auto child = parent->Child(i);
        if (child->Data(COL::Key) == key)
        {
            return child;
        }
    }
    return nullptr;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return m_rootItem->Data(section);

    return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    TreeItem *parentItem = GetItem(parent);

    TreeItem *childItem = parentItem->Child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

bool TreeModel::insertColumns(int position, int columns, const QModelIndex &parent)
{
    bool success;

    beginInsertColumns(parent, position, position + columns - 1);
    success = m_rootItem->InsertColumns(position, columns);
    endInsertColumns();

    return success;
}

bool TreeModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    TreeItem *parentItem = GetItem(parent);
    bool success;

    beginInsertRows(parent, position, position + rows - 1);
    success = parentItem->InsertChildren(position, rows, m_rootItem->ColumnCount());
    endInsertRows();

    return success;
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = GetItem(index);
    TreeItem *parentItem = childItem->Parent();

    if (parentItem == m_rootItem)
        return QModelIndex();

    return createIndex(parentItem->ChildNumber(), 0, parentItem);
}

bool TreeModel::removeColumns(int position, int columns, const QModelIndex &parent)
{
    bool success;

    beginRemoveColumns(parent, position, position + columns - 1);
    success = m_rootItem->RemoveColumns(position, columns);
    endRemoveColumns();

    if (m_rootItem->ColumnCount() == 0)
        removeRows(0, rowCount());

    return success;
}

bool TreeModel::removeRows(int position, int count, const QModelIndex &parent)
{
    TreeItem *parentItem = GetItem(parent);
    bool success = true;
    int originalCount = rowCount(parent);
    int endPosition = position + count - 1;

    beginRemoveRows(parent, position, endPosition);
    success = parentItem->RemoveChildren(position, count);
    endRemoveRows();

    if (success)
    {
        if (count == originalCount)
        {
            m_allEvents->clear();
        }
        else
        {
            for (int i = position; i <= endPosition; i++)
            {
                m_allEvents->removeAt(i);
            }
        }
    }

    return success;
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem = GetItem(parent);

    return parentItem->ChildCount();
}

bool TreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;

    TreeItem *item = GetItem(index);
    bool result = item->SetData(index.column(), value);

    if (result)
        emit dataChanged(index, index);

    return result;
}

bool TreeModel::setHeaderData(int section, Qt::Orientation orientation,
                              const QVariant &value, int role)
{
    if (role != Qt::EditRole || orientation != Qt::Horizontal)
        return false;

    bool result = m_rootItem->SetData(section, value);

    if (result)
        emit headerDataChanged(orientation, section, section);

    return result;
}

QJsonObject TreeModel::GetEvent(QModelIndex idx) const
{
    if (idx.parent().row() == -1)
    {
        return m_allEvents->at(idx.row());
    }
    else
    {
        return m_allEvents->at(idx.parent().row());
    }
}

TABTYPE TreeModel::TabType() const
{
    return m_fileType;
}

void TreeModel::SetTabType(TABTYPE type)
{
    m_fileType = type;
}

/// <summary>
/// Merge a list of new events into m_AllEvents of the model.
/// With the assumptions of both m_AllEvents and new events are already sorted on timestamps,
/// and new events are more likely to be merged to the bottom, we merge them starting from the
/// newest/bottom-most of the lists.
/// </summary>
int TreeModel::MergeIntoModelData(const EventList& events)
{
    int origIter = m_rootItem->ChildCount() - 1;
    for (int mergeIter = events.size() - 1; mergeIter >= 0; mergeIter--)
    {
        QDateTime mergeTime = QDateTime::fromString(events[mergeIter]["ts"].toString(), "yyyy-MM-ddTHH:mm:ss.zzz");
        for (; origIter >= 0; origIter--)
        {
            QDateTime origTime = m_rootItem->Child(origIter)->Data(COL::Time).toDateTime();
            if (mergeTime >= origTime)
            {
                InsertChild(origIter + 1, events[mergeIter]);
                break;
            }
        }

        if (origIter < 0)
        {
            origIter = 0;
            while (mergeIter >= 0)
            {
                InsertChild(0, events[mergeIter--]);
            }
            break;
        }
    }
    layoutChanged();

    return origIter;
}

void TreeModel::AddToModelData(const EventList& events)
{
    foreach(auto event, events)
    {
        InsertChild(m_rootItem->ChildCount(), event);
        layoutChanged();
    }
}

void TreeModel::InsertChild(int position, const QJsonObject & event)
{
    m_allEvents->insert(position, event);
    m_rootItem->InsertChildren(position, 1, m_rootItem->ColumnCount());
    auto child = m_rootItem->Child(position);
    if (!child)
    {
        qCritical() << "Cannot get item at" << position;
        return;
    }

    SetupChild(child, event);
}

void TreeModel::SetupChild(TreeItem *child, const QJsonObject & event)
{
    child->SetData(COL::ID, event["idx"].toInt());
    child->SetData(COL::File, event["file"].toString());
    child->SetData(COL::Time, QDateTime::fromString(QString(event["ts"].toString()), "yyyy-MM-ddTHH:mm:ss.zzz"));
    child->SetData(COL::PID, event["pid"].toString());
    child->SetData(COL::TID, event["tid"].toString());
    child->SetData(COL::Severity, event["sev"].toString());
    child->SetData(COL::Request, event["req"].toString());
    child->SetData(COL::Session, event["sess"].toString());
    child->SetData(COL::Site, event["site"].toString());
    child->SetData(COL::User, event["user"].toString());
    child->SetData(COL::Key, event["k"].toString());

    auto v = event["v"];
    if (!v.isObject())
    {
        child->SetData(COL::Value, v.toString().replace("\n", "\\n"));
    }
    else
    {
        QJsonObject obj = v.toObject();
        child->SetData(COL::Value, JsonToString(obj));
        AddChildren(obj, child);
    }

    // calculate "Elapsed"
    if (child->Parent() == m_rootItem)
    {
        for (int i = 0; i < child->ChildCount(); i++)
        {
            auto c = child->Child(i);
            if (c->Data(COL::Key) == "elapsed" || c->Data(COL::Key) == "created-elapsed")
            {
                auto val = c->Data(COL::Value);
                child->SetData(COL::Elapsed, val.toDouble());
                break;
            }
            else if (c->Data(COL::Key) == "elapsedMs" || c->Data(COL::Key) == "elapsed-ms")
            {
                auto val = c->Data(COL::Value);
                child->SetData(COL::Elapsed, val.toDouble() / 1000);
                break;
            }
        }
    }
}

void TreeModel::SetupModelData(TreeItem *parent)
{
    for (const auto& event : *m_allEvents)
    {
        TreeItem* child = parent->AddChild();
        SetupChild(child, event);
    }
}

void TreeModel::AddChildren(QJsonObject &obj, TreeItem *parent)
{
    for (QJsonObject::ConstIterator iter = obj.constBegin(); iter != obj.constEnd(); ++iter)
    {
        AddChild(iter.key(), iter.value(), parent);
    }
}

void TreeModel::AddChild(const QString& key, const QJsonValue& value, TreeItem* parent)
{
    TreeItem* child = parent->AddChild();
    child->SetData(COL::Key, key);

    if (value.isDouble())
    {
        double intpart;
        if (modf(value.toDouble(), &intpart) == 0)
        {
            // Note: don't use QJsonValue.toInt(), it cannot handle values outside signed 32 bits
            child->SetData(COL::Value, value.toVariant().toLongLong());
        }
        else
        {
            child->SetData(COL::Value, value.toDouble());
        }
    }
    else if (value.isObject())
    {
        QJsonObject childObj = value.toObject();
        child->SetData(COL::Value, JsonToString(childObj));
        AddChildren(childObj, child);
    }
    else if (value.isArray())
    {
        QJsonArray array = value.toArray();
        child->SetData(COL::Value, QString("%1 items").arg(array.size()));
        int itemIndex = 1;
        for (const QJsonValue& itemValue : array)
        {
            auto itemKey = QString::number(itemIndex++);
            AddChild(itemKey, itemValue, child);
        }
    }
    else
    {
        auto val = value.toVariant().toString().replace("\n", "\\n");
        child->SetData(COL::Value, val);
    }
}

QColor TreeModel::ItemHighlightColor(TreeItem * item) const
{
    if (item != nullptr)
    {
        // iterate in reverse so the rightmost tab that matches gets applied only
        for(int revItr=m_highlightOpts.count()-1; revItr>=0; revItr--)
        {
            auto highlightOpt = m_highlightOpts[revItr];
            for(auto key : highlightOpt.m_keys)
            {
                if(highlightOpt.HasMatch(item->Data(key).toString()))
                {
                    return highlightOpt.m_backgroundColor;
                }
            }
        }
    }
    return Qt::white;
}

bool TreeModel::EventHighlightMatch(const QJsonObject & event, const ColumnKeys & map)
{
    for (int revItr=m_highlightOpts.count()-1; revItr>=0; revItr--)
    {
        auto highlightOpt = m_highlightOpts[revItr];
        for(auto key : highlightOpt.m_keys)
        {
            QString search;
            if(key == COL::Value)
            {
                auto v = event["v"];
                if (!v.isObject())
                {
                    search = v.toString();
                }
                else
                {
                    QJsonObject obj = v.toObject();
                    search = JsonToString(obj);
                }
            }
            else
            {
                search = event[map[key]].toString();
            }
            if(highlightOpt.HasMatch(search))
            {
                return true;
            }
        }
    }
    return false;
}

const QString TreeModel::KeyValueString(const QString& key, const QString& value)
{
    return key % ": " % value;
}

const QString TreeModel::JsonToString(const QJsonObject& json)
{
    QVector<QString> stringList;
    GetFlatJson(json, stringList);

    if (stringList.size() == 0)
        return "";

    QString result;
    for (const QString& str : stringList)
    {
        result = result % "; " % str;
    }
    result.replace("\n", "\\n");
    return result.remove(0, 2);
}

void TreeModel::GetFlatJson(const QJsonObject& json, QVector<QString>& stringList)
{
    for (QJsonObject::ConstIterator iter = json.constBegin(); iter != json.constEnd(); ++iter)
    {
        GetFlatJson(iter.key(), iter.value(), stringList);
    }
}

void TreeModel::GetFlatJson(const QString& key, const QJsonValue& value, QVector<QString>& stringList)
{
    if (value.isDouble())
    {
        // There are no ints in JSON, only doubles
        // Check if the double value is just an integer and format accordingly
        double intpart;
        const int decimals = (modf(value.toDouble(), &intpart) == 0) ? 0 : 3;
        stringList.append(KeyValueString(key, QString::number(value.toDouble(), 'f', decimals)));
    }
    else if (value.isObject())
    {
        QJsonObject childObj = value.toObject();
        GetFlatJson(childObj, stringList);
    }
    else if (value.isArray())
    {
        QJsonArray array = value.toArray();
        int itemIndex = 1;
        stringList.append(KeyValueString(key, ""));
        for (const QJsonValue& itemValue : array)
        {
            auto itemKey = QString("%1-%2").arg(key).arg(itemIndex++);
            GetFlatJson(itemKey, itemValue, stringList);
        }
    }
    else
    {
        stringList.append(KeyValueString(key, value.toString()));
    }
}

bool TreeModel::HasFilters()
{
    return m_highlightOpts.count() > 0;
}

bool TreeModel::ValidFindOpts()
{
    return m_findOpts.m_keys.count() > 0 && m_findOpts.m_value != "";
}

void TreeModel::ClearAllEvents()
{
    m_allEvents->clear();
}

void TreeModel::ShowDeltas(qint64 delta)
{
    m_deltaBase = delta;
}

QString TreeModel::GetDeltaMSecs(QDateTime dateTime) const
{
    auto msecs = dateTime.toMSecsSinceEpoch() - m_deltaBase;
    QString formattedTime;
    bool isNegative = msecs < 0;
    msecs = msecs < 0 ? -msecs : msecs;
    int hours = msecs/(1000*60*60);
    int minutes = (msecs-(hours*1000*60*60))/(1000*60);
    int seconds = (msecs-(minutes*1000*60)-(hours*1000*60*60))/1000;
    int milliseconds = msecs-(seconds*1000)-(minutes*1000*60)-(hours*1000*60*60);

    formattedTime.append(QString("%1").arg(isNegative ? "-" : "") +
                         QString("%1").arg(hours, 2, 10, QLatin1Char('0')) + ":" +
                         QString("%1").arg(minutes, 2, 10, QLatin1Char('0')) + ":" +
                         QString("%1").arg(seconds, 2, 10, QLatin1Char('0')) + ":" +
                         QString("%1").arg(milliseconds, 3, 10, QLatin1Char('0')));

    return formattedTime;
}

bool TreeModel::IsHighlightedRow(int row) const
{
    QModelIndex idx = index(row, 0);
    TreeItem* item = GetItem(idx);
    bool highlighted = (ItemHighlightColor(item) != Qt::white);
    return highlighted;
}
