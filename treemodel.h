#include <QAbstractItemModel>
#include <QMimeData>
#include <QSqlQuery>

struct Node {
    int id { 0 };
    QString name { "" };
    QString description { "" };

    Node* parent { nullptr };
    QList<Node*> children;

    Node(int id, QString name, QString description)
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

    QVariant headerData(int section, Qt::Orientation orientation,
        int role = Qt::DisplayRole) const override;

    void sort(int column, Qt::SortOrder order) override;

    bool insertRows(int row, int count,
        const QModelIndex& parent = QModelIndex()) override;
    bool removeRows(int row, int count,
        const QModelIndex& parent = QModelIndex()) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    Qt::DropActions supportedDropActions() const override;
    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QModelIndexList& indexes) const override;
    bool canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const override;
    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row,
        int column, const QModelIndex& parent) override;

private:
    bool InsertRecord(int id_parent, QString name);
    bool UpdateRecord(int id, QString column, QString string);
    bool DeleteRecord(int id, int id_parent);
    bool DragRecord(int id, int new_parent);

    void ConstructTree(const QSqlDatabase& db);
    void ConstructLeafPaths(const QSqlDatabase& db, QChar c);

    Node* GetNode(const QModelIndex& index) const;
    Node* GetNode(Node* parent, int id);
    bool IsDescendant(Node* descendant, Node* ancestor);

private:
    Node* root;

    QSqlDatabase db;
    TableInfo table_info;

    int id;
    QStringList headers;

    QHash<int, QString> leaf_paths;
};
