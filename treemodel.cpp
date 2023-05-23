#include "treemodel.h"
#include <QDebug>
#include <QIODevice>
#include <QSqlError>

TreeModel::TreeModel(const TableInfo& table, QObject* parent)
    : QAbstractItemModel(parent)
    , root(nullptr)
    , table_info(table)
{
    db = QSqlDatabase::addDatabase("QSQLITE", "connection1");
    db.setDatabaseName(table_info.database);

    if (!db.open()) {
        qWarning() << "Failed to open database:" << db.lastError().text();
        return;
    }

    if (!db.isOpen()) {
        qWarning() << "Database is not open";
        return;
    }

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

    query.prepare(QString("SELECT descendant FROM %1 "
                          "GROUP BY descendant HAVING COUNT(descendant) = 1")
                      .arg(table_info.node_path));

    if (!query.exec()) {
        qWarning() << "Error query data from node_path 1st step"
                   << query.lastError().text();
    }

    int descendant_id = 0;
    int ancestor_id = 0;
    Node* ancestor;
    Node* descendant;

    while (query.next()) {
        ancestor = root;
        descendant_id = query.value(0).toInt();
        descendant = hash.value(descendant_id);

        if (!ancestor->HasLChild()) {
            ancestor->lchild = descendant;
            descendant->previous = ancestor;
        } else {
            ancestor = root->lchild;

            while (ancestor->HasRSibling()) {
                ancestor = ancestor->rsibling;
            }

            ancestor->rsibling = descendant;
            descendant->previous = ancestor;
        }
    }

    query.prepare(QString("SELECT ancestor, descendant FROM %1 WHERE distance = 1").arg(table_info.node_path));

    if (!query.exec()) {
        qWarning() << "Error query data from node_path 2nd"
                   << query.lastError().text();
    }

    while (query.next()) {
        ancestor_id = query.value(0).toInt();
        descendant_id = query.value(1).toInt();

        ancestor = hash.value(ancestor_id);
        descendant = hash.value(descendant_id);

        if (!ancestor->HasLChild()) {
            ancestor->lchild = descendant;
            descendant->previous = ancestor;
        } else {
            ancestor = ancestor->lchild;

            while (ancestor->HasRSibling()) {
                ancestor = ancestor->rsibling;
            }

            ancestor->rsibling = descendant;
            descendant->previous = ancestor;
        }
    }

    if (query.lastError().isValid()) {
        qWarning() << "Error construct TABLE NODE_PATH:" << query.lastError().text();
    } else if (!query.isActive()) {
        qWarning() << "TABLE NODE_PATHis not active";
    }
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    Node* node_parent = GetNode(parent);
    auto* node = GetChild(node_parent, row);

    if (node)
        return createIndex(row, column, node);

    return QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return QModelIndex();

    auto* node = static_cast<Node*>(index.internalPointer());

    while (!node->IsLChild()) {
        node = node->previous;
    }

    auto* node_parent = node->previous;
    if (node_parent == root)
        return QModelIndex();

    node = node_parent;
    int i = 0;
    while (!node->IsLChild()) {
        ++i;
        node = node->previous;
    }

    return createIndex(i, 0, node_parent);
}

int TreeModel::rowCount(const QModelIndex& parent) const
{
    int i = 0;
    auto* node_parent = GetNode(parent);
    auto* node_tmp = node_parent->lchild;

    while (node_tmp != nullptr) {
        node_tmp = node_tmp->rsibling;
        ++i;
    }

    return i;
}

QVariant TreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    auto* node = static_cast<Node*>(index.internalPointer());

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
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
    default:
        break;
    }

    return false;
}

int TreeModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return 3;
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

    emit layoutChanged();
}

Node* TreeModel::GetNode(const QModelIndex& index) const
{
    if (!index.isValid())
        return root;
    else
        return static_cast<Node*>(index.internalPointer());
}

Node* TreeModel::GetChild(Node* parent, int row) const
{
    auto* node = parent->lchild;
    int i = 0;

    while (i != row) {
        node = node->rsibling;
        ++i;
    }

    return node;
}

Node* TreeModel::GetChild(int id, Node* parent) const
{
    auto* node = parent->lchild;

    while (id != node->id)
        node = node->rsibling;

    return node;
}

bool TreeModel::insertRows(int row, int count, const QModelIndex& parent)
{
    if (count != 1)
        return false;

    auto* node_parent = root;
    if (parent.isValid())
        node_parent = GetNode(parent);

    InsertRecord(node_parent->id, "New Node");
    auto* node_new = new Node(id, "New Node", "");

    auto* node = GetChild(node_parent, row);

    beginInsertRows(parent, row, row);
    if (node == nullptr) {
        node_parent->lchild = node_new;
        node_new->previous = node_parent;
    } else if (node->IsLChild()) {
        node_parent->lchild = node_new;
        node_new->previous = node_parent;

        node->previous = node_new;
        node_new->rsibling = node;
    } else {
        node->previous->rsibling = node_new;
        node_new->previous = node->previous;

        node_new->rsibling = node;
        node->previous = node_new;
    }

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

    auto* node_parent = root;
    if (parent.isValid())
        node_parent = GetNode(parent);

    auto* node = GetChild(node_parent, row);
    int id = node->id;

    beginRemoveRows(parent, row, row);

    if (node->HasLChild()) {
        auto* tmp = node;

        while (tmp->HasRSibling()) {
            tmp = tmp->rsibling;
        }

        tmp->rsibling = node->lchild;
        node->lchild->previous = tmp;

        node->lchild = nullptr;
    }

    if (!node->IsLChild()) {
        if (!node->HasRSibling()) {
            node->previous->rsibling = nullptr;
        } else {
            node->previous->rsibling = node->rsibling;
            node->rsibling->previous = node->previous;
        }
    } else {
        if (!node->HasRSibling()) {
            node->previous->lchild = nullptr;
        } else {
            node->previous->lchild = node->rsibling;
            node->rsibling->previous = node->previous;
        }
    }

    delete node;
    node = nullptr;

    endRemoveRows();

    DeleteRecord(id);

    return true;
}

bool TreeModel::DeleteRecord(int id)
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
        "(SELECT descendant FROM %1 WHERE ancestor = :id and distance !=0)")
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

bool TreeModel::SortRecord()
{
    return true;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return QStringLiteral("Column %1").arg(section);

    return QVariant();
}

Qt::ItemFlags TreeModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    auto default_flags = QAbstractItemModel::flags(index);

    if (index.column() == 0 || index.column() == 1)
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | default_flags;
    else
        return Qt::ItemIsEditable | default_flags;
}

Qt::DropActions TreeModel::supportedDropActions() const
{
    return Qt::MoveAction | Qt::CopyAction;
}

QStringList TreeModel::mimeTypes() const
{
    QStringList types;
    types << "application/x-qabstractitemmodeldatalist";
    return types;
}

QMimeData* TreeModel::mimeData(const QModelIndexList& indexes) const
{
    QMimeData* data_mime = new QMimeData();

    QByteArray data_encoded;
    QDataStream stream(&data_encoded, QIODevice::WriteOnly);

    for (const QModelIndex& index : indexes) {
        if (index.isValid()) {
            Node* node = GetNode(index);
            stream << node->id;
        }
    }

    data_mime->setData("application/x-qabstractitemmodeldatalist", data_encoded);
    return data_mime;
}

bool TreeModel::IsDescendantOf(Node* possibleDescendant, Node* possibleAncestor) const
{
    if (!possibleDescendant || !possibleAncestor) {
        return false;
    }

    //    Node* currentNode = possibleDescendant->previous;
    //    while (currentNode) {
    //        if (currentNode == possibleAncestor) {
    //            return true;
    //        }
    //        currentNode = currentNode->previous;
    //    }
    return false;
}

bool TreeModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row,
    int column, const QModelIndex& parent)
{
    if (action == Qt::IgnoreAction)
        return true;

    if (!data->hasFormat("application/x-qabstractitemmodeldatalist"))
        return false;

    if (column != 0)
        return false;

    QByteArray data_encoded = data->data("application/x-qabstractitemmodeldatalist");
    QDataStream stream(&data_encoded, QIODevice::ReadOnly);

    QList<int> newItems;
    int newItem;

    while (!stream.atEnd()) {
        stream >> newItem;
        newItems << newItem;
    }

    auto* node_parent = root;
    if (parent.isValid())
        node_parent = GetNode(parent);

    //    int beginRow = row == -1 ? node_parent->children.count() : row;

    Node* moved_node;

    for (int itemId : newItems) {
        moved_node = GetChild(itemId, node_parent);
        if (moved_node) {
            // 如果目标父节点与移动节点的当前父节点相同，跳过此节点
            //            if (moved_node->parent == node_parent || IsDescendantOf(node_parent, moved_node)) {
            //                continue;
            //            }
            //            QModelIndex movedIndex = createIndex(
            //                moved_node->parent->children.indexOf(moved_node), 0, moved_node);
            //            beginRemoveRows(movedIndex.parent(), movedIndex.row(), movedIndex.row());
            //            moved_node->parent->children.removeOne(moved_node);
            //            endRemoveRows();

            //            beginInsertRows(parent, beginRow, beginRow);
            //            node_parent->children.insert(beginRow, moved_node);
            //            moved_node->parent = node_parent;
            endInsertRows();
        }
    }

    return true;
}
