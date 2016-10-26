#ifndef TREEMODEL_H
#define TREEMODEL_H

#include "colorlibrary.h"
#include "highlightoptions.h"
#include "searchopt.h"

#include <memory>
#include <QAbstractItemModel>
#include <QColor>
#include <QHash>
#include <QJsonObject>
#include <QModelIndex>
#include <QVariant>
#include <queue>
#include <utility>
#include <vector>

class TreeItem;
typedef QHash<COL, QString> ColumnKeys;
typedef QList<QJsonObject> EventList;
typedef std::shared_ptr<EventList> EventListPtr;

enum class TABTYPE {
    SingleFile = 0,
    Directory,
    ExportedEvents
};

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    TreeModel(const QStringList &headers, const EventListPtr events, QObject *parent = 0);
    ~TreeModel();

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

    bool insertColumns(int position, int columns, const QModelIndex &parent = QModelIndex()) override;
    bool removeColumns(int position, int columns, const QModelIndex &parent = QModelIndex()) override;
    bool insertRows(int position, int rows, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex()) override;

    QString GetChildValueString(const QModelIndex &index, QString key) const;
    int MergeIntoModelData(const EventList& events);
    void AddToModelData(const EventList& events);
    bool HasFilters();
    bool ValidFindOpts();
    void ClearAllEvents();
    void ShowDeltas(qint64 delta);
    bool IsHighlightedRow(int row) const;
    QJsonObject GetEvent(QModelIndex idx) const;
    QString GetValueFullString(const QModelIndex& idx, bool singleLineFormat = false) const;
    TABTYPE TabType() const;
    void SetTabType(TABTYPE type);

    bool m_highlightOnlyMode;
    bool m_liveMode;
    HighlightOptions m_highlightOpts;
    ColorLibrary m_colorLibrary;
    SearchOpt m_findOpts;
    QList<QString> m_paths;

private:
    void SetupModelData(TreeItem *parent);
    void SetupChild(TreeItem *parent, const QJsonObject & event);
    void AddChildren(QJsonObject &obj, TreeItem *parent);
    void AddChild(const QString& key, const QJsonValue& value, TreeItem* parent);
    void InsertChild(int position, const QJsonObject & event);
    const QString KeyValueString(const QString& key, const QString& value) const;
    QString JsonToString(const QJsonObject& json, const QString& lineBreak = "; ") const;
    void GetFlatJson(const QJsonObject& json, QVector<QString>& stringList) const;
    void GetFlatJson(const QString& key, const QJsonValue& value, QVector<QString>& stringList) const;
    QColor ItemHighlightColor(const QModelIndex& idx) const;
    QString GetDeltaMSecs(QDateTime dateTime) const;
    TreeItem *GetItem(const QModelIndex &index) const;

    TreeItem * m_rootItem;
    qint64 m_deltaBase = 0;

    // Used by ToggleHighlightOnly, as not all events might be shown.
    EventListPtr m_allEvents;
    TABTYPE m_fileType;
};

#endif // TREEMODEL_H
