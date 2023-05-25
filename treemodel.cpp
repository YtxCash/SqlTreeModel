#include "treemodel.h"
#include <QDebug>
#include <QIODevice>
#include <QSqlError>

TreeModel::TreeModel(const TableInfo& table, QObject* parent)
    : QAbstractItemModel(parent)
    , root(nullptr)
    , table_info(table)
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(table_info.database);

    if (!db.open()) {
        qWarning() << "Failed to open database:" << db.lastError().text();
        return;
    }

    if (!db.isOpen()) {
        qWarning() << "Database is not open";
        return;
    }

    headers << "Account"
            << "Id"
            << "Description";

    ConstructTree();
}

TreeModel::~TreeModel()
{
    delete root;
    db.close();
}

void TreeModel::ConstructTree()
{

    auto query = QSqlQuery(db);

    query.prepare(QString("SELECT id, name, description FROM %1").arg(table_info.node));

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

    query.prepare(QString("SELECT ancestor, descendant FROM %1 WHERE distance = 1").arg(table_info.node_path));
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
    if (!index.isValid())
        return QVariant();

    auto* node = static_cast<Node*>(index.internalPointer());

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0:
            return node->name;
            break;
        case 1:
            return node->id;
            break;
        case 2:
            return node->description;
            break;
        default:
            break;
        }
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
    query.prepare(QString("UPDATE %1 SET %2 = :string WHERE id = :id").arg(table_info.node, column));
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

    std::function<bool(Node*, Node*)> Compare =
        [column, order](Node* lhs, Node* rhs) -> bool {
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

    std::function<void(Node*)> SortChildren =
        [&Compare, &SortChildren](Node* node) {
            std::sort(node->children.begin(), node->children.end(), Compare);

            for (Node* child : node->children) {
                SortChildren(child);
            }
        };

    SortChildren(root);

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

Node* TreeModel::FindNode(Node* parent, int id)
{
    if (!parent)
        return nullptr;

    for (Node* child : parent->children) {
        if (child->id == id)
            return child;

        Node* node = FindNode(child, id);
        if (node)
            return node;
    }

    return nullptr;
}

bool TreeModel::insertRows(int row, int count, const QModelIndex& parent)
{
    if (count != 1)
        return false;

    auto* node_parent = GetNode(parent);

    InsertRecord(node_parent->id, "New Node");
    auto* new_node = new Node(id, "New Node", "");

    beginInsertRows(parent, row, row);

    new_node->parent = node_parent;
    node_parent->children.insert(row, new_node);

    endInsertRows();

    return true;
}

bool TreeModel::InsertRecord(int id_parent, QString name)
{

    QSqlQuery query = QSqlQuery(db);

    query.prepare(QString("INSERT INTO %1 (name) VALUES (:name)").arg(table_info.node));
    query.bindValue(":name", name);

    if (!query.exec()) {
        qWarning() << "Failed to add node" << query.lastError().text();
        return false;
    }

    id = query.lastInsertId().toInt();

    query.prepare(QString(
        "INSERT INTO %1 (ancestor, descendant, distance) "
        "SELECT ancestor, :descendant, distance + 1 "
        "FROM node_path WHERE descendant = :parent "
        "UNION ALL SELECT :descendant, :descendant, 0")
                      .arg(table_info.node_path));
    query.bindValue(":descendant", id);
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
            node_parent->children.append(child);
        }
    }

    node_parent->children.removeOne(node);
    delete node;

    endRemoveRows();

    DeleteRecord(id);

    return true;
}

bool TreeModel::DeleteRecord(const int id)
{
    QSqlQuery query = QSqlQuery(db);

    query.prepare(QString("DELETE FROM %1 WHERE id = :id").arg(table_info.node));
    query.bindValue(":id", id);
    if (!query.exec()) {
        qWarning() << "Failed to remove node 1st step" << query.lastError().text();
        return false;
    }

    query.prepare(QString(
        "UPDATE %1 "
        "SET distance = distance -1 "
        "WHERE distance != 0 AND descendant IN "
        "(SELECT descendant FROM %1 WHERE ancestor = :id AND distance !=0)")
                      .arg(table_info.node_path));
    query.bindValue(":id", id);
    if (!query.exec()) {
        qWarning() << "Failed to remove node_path 2nd step"
                   << query.lastError().text();
        return false;
    }

    query.prepare(QString(
        "DELETE FROM %1 "
        "WHERE descendant = :id OR ancestor = :id")
                      .arg(table_info.node_path));
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
                      .arg(table_info.node_path));
    query.bindValue(":id", id);
    if (!query.exec()) {
        qWarning() << "Failed to drag node_path 1st step"
                   << query.lastError().text();
        return false;
    }

    query.prepare(QString("INSERT INTO %1 (ancestor, descendant, distance) "
                          "SELECT p.ancestor ancestor, s.descendant descendant, p.distance + s.distance + 1 distance "
                          "FROM %1 p "
                          "CROSS JOIN %1 s "
                          "WHERE p.descendant = :new_parent AND s.ancestor = :id")
                      .arg(table_info.node_path));
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
    return Qt::MoveAction;
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
    int begin_row = row == -1 ? 0 : row;

    Node* node;

    for (int id : ids) {
        node = FindNode(root, id);
        if (node) {
            if (node->parent == node_parent) {
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
