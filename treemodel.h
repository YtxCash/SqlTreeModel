#include <QAbstractItemModel>
#include <QMimeData>
#include <QSqlQuery>

struct Node {

    Node(int id, QString name, QString description)
        : id(id)
        , name(name)
        , description(description)
    {
    }

    bool IsLChild()
    {
        return previous != nullptr && previous->lchild == this;
    }

    bool HasRSibling()
    {
        return rsibling != nullptr;
    }

    bool HasLChild()
    {
        return lchild != nullptr;
    }

    int id { 0 };
    QString name { "" };
    QString description { "" };

    Node* previous { nullptr };
    Node* lchild { nullptr };
    Node* rsibling { nullptr };

#if 0 // 暂时用不上
    Node* GetLChild()
    {
        return lchild;
    }

    bool HasPrevious()
    {
        return previous != nullptr;
    };

    bool IsLeaf()
    {
        return lchild == nullptr && rsibling == nullptr;
    }

    bool IsRSibling()
    {
        return previous != nullptr && previous->rsibling == this;
    }
#endif
};

struct TableInfo {
    QString database { "" };
    QString node { "" };
    QString node_path { "" };

    TableInfo(QString database, QString node, QString node_path)
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

#if 0 // 暂时不考虑拖动
    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QModelIndexList& indexes) const override;
    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row,
        int column, const QModelIndex& parent) override;
    bool IsDescendantOf(Node* possibleDescendant, Node* possibleAncestor);
#endif

private:
    bool InsertRecord(int id_parent, QString name);
    bool UpdateRecord(int id, QString column, QString string);
    bool DeleteRecord(int id);
    bool SortRecord();

    void ConstructTree();

    Node* GetNode(const QModelIndex& index) const;
    Node* GetNode(Node* parent, int row) const;

private:
    Node* root;

    QSqlDatabase db;
    TableInfo table_info;
};
