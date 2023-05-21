#include <QAbstractItemModel>
#include <QMimeData>
#include <QSqlQuery>

class Node {

public:
    Node(const int& id, const QString& name, const QString& description);

public:
    bool HasPrevious()
    {
        return previous != nullptr;
    };

    bool HasChild()
    {
        return child != nullptr;
    }

    bool HasSibling()
    {
        return sibling != nullptr;
    }

    bool IsLeaf()
    {
        return child == nullptr && sibling == nullptr;
    }

    bool IsChild()
    {
        return previous != nullptr && previous->child == this;
    }

    bool IsSibling()
    {
        return previous != nullptr && previous->sibling == this;
    }

public:
    QChar mark { '\0' };
    int id { 0 };
    QString name { "" };
    QString description { "" };

public:
    Node* previous { nullptr };
    Node* child { nullptr };
    Node* sibling { nullptr };
};

struct TableInfo {
    QString database { "" };
    QString node { "" };
    QString node_path { "" };

    TableInfo(const QString& database, const QString& node, const QString& node_path)
        : database(database)
        , node(node)
        , node_path(node_path)
    {
    }
};

class TreeModel : public QAbstractItemModel {
public:
    TreeModel(const TableInfo& table_info, QObject* parent = nullptr);
    ~TreeModel();

public:
    QModelIndex index(int row, int column,
        const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index,
        int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value,
        int role = Qt::EditRole) override;

    void sort(int column, Qt::SortOrder order) override;

    bool insertRows(int row, int count,
        const QModelIndex& parent = QModelIndex()) override;
    bool removeRows(int row, int count,
        const QModelIndex& parent = QModelIndex()) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // 暂时不考虑拖动
#if 0
    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QModelIndexList& indexes) const override;
    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row,
        int column, const QModelIndex& parent) override;
    bool IsDescendantOf(Node* possibleDescendant, Node* possibleAncestor);
#endif

private:
    bool InsertRecord(const int id_parent, const QString& name);
    bool UpdateRecord(const int id, const QString& column, const QString& name);
    bool DeleteRecord(const int id);
    bool SortRecord();

    void ConstructTree();

    Node* GetNode(const QModelIndex& index) const;
    Node* FindChild(Node* parent, int row) const;

private:
    Node* node_root;

    QSqlDatabase db;
    TableInfo table_info;
};
