#include "treemodel.h"
#include <QDebug>
#include <QIODevice>
#include <QMimeData>
#include <QSqlError>
#include <QSqlQuery>

TreeModel::TreeModel(const QSqlDatabase& db, const TreeInfo& tree_info, QObject* parent)
    : QAbstractItemModel { parent }
    , root { nullptr }
    , tree_info { tree_info }
    , db { db }
{
    headers << "Account"
            << "Id"
            << "Description";

    ConstructTree(db);
    ConstructLeafPaths(db, separator);
}

TreeModel::~TreeModel()
{
    delete root;
}

void TreeModel::ConstructTree(const QSqlDatabase& db)
{

    auto query = QSqlQuery(db);

    query.prepare(QString("SELECT id, name, description FROM %1").arg(tree_info.node));

    if (!query.exec()) {
        qWarning() << "Error query data from node"
                   << query.lastError().text();
    }

    root = new Node(-1, "root", "");

    QHash<int, Node*> hash;

    int id = 0;
    QString name;
    QString description;

    while (query.next()) {
        id = query.value(0).toInt();
        name = query.value(1).toString();
        description = query.value(2).toString();
        hash[id] = new Node(id, name, description);
    }

    if (query.lastError().isValid()) {
        qWarning() << "Error construct TABLE NODE:" << query.lastError().text();
    } else if (!query.isActive()) {
        qWarning() << "TABLE NODE is not active";
    }

    query.prepare(QString("SELECT ancestor, descendant FROM %1 WHERE distance = 1").arg(tree_info.node_path));
    if (!query.exec()) {
        qWarning() << "Error query data from node path"
                   << query.lastError().text();
    }

    int descendant_id = 0;
    int ancestor_id = 0;
    Node* ancestor;
    Node* descendant;

    while (query.next()) {
        ancestor_id = query.value(0).toInt();
        descendant_id = query.value(1).toInt();

        ancestor = hash.value(ancestor_id);
        descendant = hash.value(descendant_id);

        if (ancestor && descendant) {
            ancestor->children.emplace_back(descendant);
            descendant->parent = ancestor;
        }
    }

    if (query.lastError().isValid()) {
        qWarning() << "Error construct TABLE NODE_PATH:" << query.lastError().text();
    } else if (!query.isActive()) {
        qWarning() << "TABLE NODE_PATHis not active";
    }

    for (auto* node : qAsConst(hash)) {
        if (!node->parent) {
            node->parent = root;
            root->children.emplace_back(node);
        }
    }
}

void TreeModel::ConstructLeafPaths(const QSqlDatabase& db, QChar c)
{
    leaf_paths.clear();

    auto query = QSqlQuery(db);
    query.prepare(QString("SELECT n1.id FROM %1 n1 "
                          "INNER JOIN %2 n2 ON n1.id = n2.ancestor "
                          "GROUP BY n1.id, n1.name "
                          "HAVING COUNT(n2.ancestor) = 1;")
                      .arg(tree_info.node, tree_info.node_path));
    if (!query.exec()) {
        qWarning() << "Error query data from node path"
                   << query.lastError().text();
    }

    Node* node;
    while (query.next()) {

        int id = query.value(0).toInt();
        node = GetNode(root, id);

        QString path = node->name;

        while (node->parent != root) {
            node = node->parent;
            path = node->name + c + path;
        }

        leaf_paths[path] = id;
    }
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    auto* node_parent = GetNode(parent);
    auto* node = node_parent->children.value(row);

    if (node)
        return createIndex(row, column, node);

    return QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return QModelIndex();

    auto* node = static_cast<Node*>(index.internalPointer());

    if (node == root)
        return QModelIndex();

    auto* node_parent = node->parent;

    if (node_parent == root)
        return QModelIndex();

    return createIndex(node_parent->parent->children.indexOf(node_parent), 0,
        node_parent);
}

int TreeModel::rowCount(const QModelIndex& parent) const
{
    auto* node_parent = GetNode(parent);

    return node_parent->children.size();
}

QVariant TreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* node = static_cast<Node*>(index.internalPointer());

    switch (index.column()) {
    case 0:
        return node->name;
    case 1:
        return node->id;
    case 2:
        return node->description;
    default:
        return QVariant();
    }

    return QVariant();
}

bool TreeModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    auto* node = static_cast<Node*>(index.internalPointer());

    switch (index.column()) {
    case 0:
        break;
    case 1:
        break;
    case 2:
        node->description = value.toString();
        emit dataChanged(index, index, QVector<int>() << role);
        UpdateRecord(node->id, "description", value.toString());
        return true;
    }

    return false;
}

int TreeModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return headers.size();
}

bool TreeModel::UpdateRecord(int id, QString column, QString string)
{

    QSqlQuery query = QSqlQuery(db);
    query.prepare(QString("UPDATE %1 SET %2 = :string WHERE id = :id").arg(tree_info.node, column));
    query.bindValue(":id", id);
    query.bindValue(":string", string);

    if (!query.exec()) {
        qWarning() << "Failed to edit record:" << query.lastError().text();
        return false;
    }
    return true;
}

void TreeModel::sort(int column, Qt::SortOrder order)
{
    emit layoutAboutToBeChanged();

    auto Compare = [column, order](const Node* lhs, const Node* rhs) -> bool {
        bool result;
        switch (column) {
        case 0:
            result = lhs->name < rhs->name;
            break;
        case 1:
            result = lhs->id < rhs->id;
            break;
        case 2:
            result = lhs->description < rhs->description;
            break;
        default:
            result = false;
            break;
        }

        return order == Qt::AscendingOrder ? result : !result;
    };

    std::function<void(Node*)> Sort =
        [&Compare, &Sort](Node* node) {
            std::sort(node->children.begin(), node->children.end(), Compare);

            for (Node* child : node->children) {
                Sort(child);
            }
        };

    Sort(root);

    emit layoutChanged();
}

Node* TreeModel::GetNode(const QModelIndex& index) const
{
    if (index.isValid()) {
        auto* node = static_cast<Node*>(index.internalPointer());
        if (node)
            return node;
    }

    return root;
}

Node* TreeModel::GetNode(Node* parent, int id)
{
    if (!parent)
        return nullptr;

    for (Node* child : parent->children) {
        if (child->id == id)
            return child;

        Node* node = GetNode(child, id);
        if (node)
            return node;
    }

    return nullptr;
}

bool TreeModel::IsDescendant(Node* descendant, Node* ancestor)
{
    if (!descendant || !ancestor) {
        return false;
    }

    Node* node = descendant->parent;

    while (node) {
        if (node == ancestor) {
            return true;
        }
        node = node->parent;
    }

    return false;
}

void TreeModel::UpdateLeafPaths()
{
    emit LeafPaths(leaf_paths);
}

bool TreeModel::insertRows(int row, int count, const QModelIndex& parent)
{
    if (count != 1)
        return false;

    auto* node_parent = GetNode(parent);

    InsertRecord(node_parent->id, "New Node");
    auto* new_node = new Node(id_last_insert, "New Node", "");

    beginInsertRows(parent, row, row);

    new_node->parent = node_parent;
    node_parent->children.insert(row, new_node);

    endInsertRows();

    ConstructLeafPaths(db, separator);
    UpdateLeafPaths();

    return true;
}

bool TreeModel::InsertRecord(int id_parent, QString name)
{

    auto query = QSqlQuery(db);

    query.prepare(QString("INSERT INTO %1 (name) VALUES (:name)").arg(tree_info.node));
    query.bindValue(":name", name);

    if (!query.exec()) {
        qWarning() << "Failed to add node" << query.lastError().text();
        return false;
    }

    id_last_insert = query.lastInsertId().toInt();

    query.prepare(QString(
        "INSERT INTO %1 (ancestor, descendant, distance) "
        "SELECT ancestor, :id, distance + 1 "
        "FROM %1 WHERE descendant = :parent "
        "UNION ALL SELECT :id, :id, 0")
                      .arg(tree_info.node_path));
    query.bindValue(":id", id_last_insert);
    query.bindValue(":parent", id_parent);

    if (!query.exec()) {
        qWarning() << "Failed to add node_path"
                   << query.lastError().text();
        return false;
    }
    return true;
}

bool TreeModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (row < 0 || count != 1)
        return false;

    auto* node_parent = GetNode(parent);
    Node* node = node_parent->children.at(row);
    int id = node->id;

    beginRemoveRows(parent, row, row);

    if (!node_parent->children.isEmpty()) {
        for (Node* child : node->children) {
            child->parent = node_parent;
            node_parent->children.emplace_back(child);
        }
    }

    node_parent->children.removeOne(node);
    delete node;
    node = nullptr;

    endRemoveRows();

    DeleteRecord(id, node_parent->id);

    ConstructLeafPaths(db, separator);
    UpdateLeafPaths();

    return true;
}

bool TreeModel::DeleteRecord(int id, int id_parent)
{
    QSqlQuery query = QSqlQuery(db);

    query.prepare(QString("DELETE FROM %1 WHERE id = :id").arg(tree_info.node));
    query.bindValue(":id", id);
    if (!query.exec()) {
        qWarning() << "Failed to remove node 1st step" << query.lastError().text();
        return false;
    }

    query.prepare(QString(
        "UPDATE %1 SET distance = distance -1 WHERE "
        "(descendant IN (SELECT descendant FROM %1 WHERE ancestor = :id AND ancestor != descendant) "
        "AND ancestor IN (SELECT ancestor FROM %1 WHERE descendant = :id AND ancestor != descendant))")
                      .arg(tree_info.node_path));
    query.bindValue(":id", id);
    if (!query.exec()) {
        qWarning() << "Failed to remove node_path 2nd step"
                   << query.lastError().text();
        return false;
    }

    query.prepare(QString(
        "DELETE FROM %1 "
        "WHERE descendant = :id OR ancestor = :id")
                      .arg(tree_info.node_path));
    query.bindValue(":id", id);
    if (!query.exec()) {
        qWarning() << "Failed to remove node_path 3rd step"
                   << query.lastError().text();
        return false;
    }

    return true;
}

bool TreeModel::DragRecord(int id, int new_parent)
{
    QSqlQuery query = QSqlQuery(db);

    query.prepare(QString("DELETE FROM %1 WHERE "
                          "(descendant IN (SELECT descendant FROM %1 WHERE ancestor = :id) AND "
                          "ancestor IN (SELECT ancestor FROM %1 WHERE descendant = :id AND ancestor != descendant))")
                      .arg(tree_info.node_path));
    query.bindValue(":id", id);
    if (!query.exec()) {
        qWarning() << "Failed to drag node_path 1st step"
                   << query.lastError().text();
        return false;
    }

    query.prepare(QString("INSERT INTO %1 (ancestor, descendant, distance) "
                          "SELECT p.ancestor, s.descendant, p.distance + s.distance + 1 "
                          "FROM %1 p "
                          "CROSS JOIN %1 s "
                          "WHERE p.descendant = :new_parent AND s.ancestor = :id")
                      .arg(tree_info.node_path));
    query.bindValue(":id", id);
    query.bindValue(":new_parent", new_parent);

    if (!query.exec()) {
        qWarning() << "Failed to drag node_path 2nd step"
                   << query.lastError().text();
        return false;
    }

    return true;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return headers.at(section);

    return QVariant();
}

Qt::ItemFlags TreeModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    auto default_flags = QAbstractItemModel::flags(index);

    switch (index.column()) {
    case 0:
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | default_flags;
        break;
    case 1:
        return default_flags;
        break;
    default:
        return Qt::ItemIsEditable | default_flags;
        break;
    }
}

Qt::DropActions TreeModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

QStringList TreeModel::mimeTypes() const
{
    QStringList types;
    types << "application/id";
    return types;
}

QMimeData* TreeModel::mimeData(const QModelIndexList& indexes) const
{
    QMimeData* data_mime = new QMimeData();
    QByteArray data_encoded;

    QDataStream stream(&data_encoded, QIODevice::WriteOnly);

    QList<Node*> nodes;

    for (const QModelIndex& index : indexes) {
        if (index.isValid()) {
            auto* node = static_cast<Node*>(index.internalPointer());
            if (!nodes.contains(node))
                nodes << node;
        }
    }

    for (const Node* node : nodes) {
        stream << node->id;
    }

    data_mime->setData("application/id", data_encoded);
    return data_mime;
}

bool TreeModel::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const
{
    Q_UNUSED(row);
    Q_UNUSED(column);
    Q_UNUSED(parent);

    if (action == Qt::IgnoreAction || !data->hasFormat("application/id"))
        return false;

    return true;
}

bool TreeModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row,
    int column, const QModelIndex& parent)
{
    if (!canDropMimeData(data, action, row, column, parent))
        return false;

    QByteArray encodedData = data->data("application/id");
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    QList<int> ids;
    int id;

    while (!stream.atEnd()) {
        stream >> id;
        ids << id;
    }

    Node* node_parent = GetNode(parent);
    int begin_row = row == -1 ? node_parent->children.size() : row;

    Node* node;

    for (int id : ids) {
        node = GetNode(root, id);
        if (node) {
            if (node->parent == node_parent || IsDescendant(node_parent, node)) {
                continue;
            }

            QModelIndex index = createIndex(
                node->parent->children.indexOf(node), 0, node);

            beginRemoveRows(index.parent(), index.row(), index.row());
            node->parent->children.removeOne(node);
            endRemoveRows();

            beginInsertRows(parent, begin_row, begin_row);
            node_parent->children.insert(begin_row, node);
            node->parent = node_parent;
            endInsertRows();
        }

        DragRecord(id, node_parent->id);
    }

    return true;
}

QMap<QString, int> TreeModel::GetLeafPaths()
{
    return leaf_paths;
}
