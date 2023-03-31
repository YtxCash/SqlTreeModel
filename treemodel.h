#include <QAbstractItemModel>
#include <QMimeData>
#include <QSqlQuery>

struct Node {
    int id { 0 };
    QChar mark { '\0' };
    QString name { "" };
    QString description { "" };

    Node* previous { nullptr };
    Node* left_child { nullptr };
    Node* right_sibling { nullptr };

    Node(const int& id, const QString& name, const QString& description,
        Node* parent = nullptr, Node* left_child = nullptr, Node* right_sibling = nullptr)
        : id(id)
        , name(name)
        , description(description)
    {
    }
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

    Qt::DropActions supportedDropActions() const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QModelIndexList& indexes) const override;
    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row,
        int column, const QModelIndex& parent) override;

private:
    bool InsertRecord(const int id_parent, const QString& name);
    bool UpdateRecord(const int id, const QString& column, const QString& name);
    bool DeleteRecord(const int id);
    bool SortRecord();
    bool DragRecord();

    void ConstructTree();

    Node* FindNode(Node* node, const QString& name) const;

    //    QModelIndexList Find(const QString& name, int column,
    //        const QModelIndex& start, int flags);

    Node* GetNode(const QModelIndex& index) const;
    Node* FindChild(Node* parent, int row) const;

    //    bool IsDescendantOf(Node* possibleDescendant, Node* possibleAncestor);

private:
    Node* node_root;

    QSqlDatabase db;
    TableInfo table_info;
};
