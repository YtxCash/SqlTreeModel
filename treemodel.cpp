#include "treemodel.h"
#include <QDebug>
#include <QIODevice>
#include <QSqlError>

TreeModel::TreeModel(const TableInfo& table, QObject* parent)
    : QAbstractItemModel(parent)
    , node_root(nullptr)
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
    delete node_root;
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

    node_root = new Node(-1, "root", "");

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
    } else {
        qDebug() << "Construct TABLE NODE successfully";
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
        ancestor = node_root;
        descendant_id = query.value(0).toInt();
        descendant = hash.value(descendant_id);

        if (ancestor->left_child == nullptr) {
            ancestor->left_child = descendant;
            descendant->previous = ancestor;
            descendant->mark = 'z';
        } else {
            ancestor = node_root->left_child;

            while (ancestor->right_sibling != nullptr) {
                ancestor = ancestor->right_sibling;
            }

            ancestor->right_sibling = descendant;
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

        if (ancestor->left_child == nullptr) {
            ancestor->left_child = descendant;
            descendant->previous = ancestor;
            descendant->mark = 'z';
        } else {
            ancestor = ancestor->left_child;

            while (ancestor->right_sibling != nullptr) {
                ancestor = ancestor->right_sibling;
            }

            ancestor->right_sibling = descendant;
            descendant->previous = ancestor;
        }
    }

    if (query.lastError().isValid()) {
        qWarning() << "Error construct TABLE NODE_PATH:" << query.lastError().text();
    } else if (!query.isActive()) {
        qWarning() << "TABLE NODE_PATHis not active";
    } else {
        qDebug() << "Construct TABLE NODE_PATH successfully";
    }
}

Node* TreeModel::GetNode(const QModelIndex& index) const
{
    if (index.isValid()) {
        Node* node = static_cast<Node*>(index.internalPointer());
        if (node)
            return node;
    }

    return node_root;
}

Node* TreeModel::FindChild(Node* parent, int row) const
{
    auto* node_tmp = parent->left_child;
    int i = 0;

    while (i != row) {
        node_tmp = node_tmp->right_sibling;
        ++i;
    }

    return node_tmp;
}

Node* TreeModel::FindNode(Node* node, const QString& name) const
{
    if (node == nullptr)
        return nullptr;
    else if (name == node->name)
        return node;

    FindNode(node->left_child, name);
    FindNode(node->right_sibling, name);

    return nullptr;
}

// QModelIndexList TreeModel::Find(const QString& text, int column,
//     const QModelIndex& start, int flags)
//{
//     return match(start, column, text, -1, static_cast<Qt::MatchFlags>(flags));
// }

QModelIndex TreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    auto* node_parent = GetNode(parent);
    auto* node_tmp = node_parent->left_child;
    int i = 0;

    if (node_tmp == nullptr)
        return QModelIndex();

    while (i != row) {
        ++i;
        node_tmp = node_tmp->right_sibling;
    }

    return createIndex(row, column, node_tmp);
}

QModelIndex TreeModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return QModelIndex();

    auto* node = GetNode(index);

    while (node->mark != 'z') {
        node = node->previous;
    }

    auto* parent = node->previous;
    if (parent == node_root)
        return QModelIndex();

    auto* parent_tmp = parent;
    int i = 0;
    while (parent_tmp->mark != 'z') {
        ++i;
        parent_tmp = parent_tmp->previous;
    }

    return createIndex(i, 0, parent);
}

int TreeModel::rowCount(const QModelIndex& parent) const
{
    int i = 0;
    auto* node_parent = GetNode(parent);
    auto* node_tmp = node_parent->left_child;

    //    if (node_tmp == nullptr)
    //        return i;

    while (node_tmp != nullptr) {
        node_tmp = node_tmp->right_sibling;
        ++i;
    }

    return i;
}

QVariant TreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    Node* node = GetNode(index);

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
    if (role != Qt::EditRole)
        return false;

    Node* node = GetNode(index);

    switch (index.column()) {
    case 0:
        break;
    case 1:
        break;
    case 2:
        node->description = value.toString();
        if (UpdateRecord(node->id, "description", value.toString())) {
            emit dataChanged(index, index, QVector<int>() << role);
            return true;
        }
        break;
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

bool TreeModel::UpdateRecord(const int id, const QString& column, const QString& name)
{

    QSqlQuery query = QSqlQuery(db);
    query.prepare(QString("UPDATE %1 SET %2 = :name WHERE id = :id").arg(table_info.node, column));
    query.bindValue(":id", id);
    query.bindValue(":name", name);

    if (!query.exec()) {
        qWarning() << "Failed to edit record:" << query.lastError().text();
        return false;
    }
    return true;
}

void TreeModel::sort(int column, Qt::SortOrder order)
{
//    emit layoutAboutToBeChanged();
#if 0
    QModelIndexList model_indexes = persistentIndexList();
    QList<QPersistentModelIndex> persistent_indexes;
    for (const auto& model_index : model_indexes) {
        // The persistent indexes are stored in a separate list so that we can
        // change the persistent indexes in the model without changing the
        // model indexes.
        persistent_indexes.append(QPersistentModelIndex(model_index));
    }

    // The persistent indexes are sorted according to the order of the model
    // indexes. This is necessary because the model indexes are sorted
    // according to the order of the persistent indexes.
    QList<QModelIndex> source_indexes;
    for (const auto& persistent_index : persistent_indexes) {
        source_indexes.append(persistent_index);
    }

    // The nodes are sorted according to the column and order. The nodes are
    // sorted in-place.
    std::function<bool(Node*, Node*)> compare =
        [this, column, order](Node* lhs, Node* rhs) -> bool {
        bool result = (column == 0) ? (lhs->name < rhs->name)
                                    : (lhs->id < rhs->id);
        return order == Qt::AscendingOrder ? result : !result;
    };
    std::function<void(Node*)> sort_children = [&compare,
                                                   &sort_children](Node* node) {
        std::sort(node->children.begin(), node->children.end(), compare);
        for (Node* child : node->children) {
            sort_children(child);
        }
    };
    sort_children(node_root);

    // The model indexes are changed to match the sorted persistent indexes.
    for (int i = 0; i < source_indexes.size(); ++i) {
        changePersistentIndex(source_indexes.at(i), persistent_indexes.at(i));
    }
#endif
    //    emit layoutChanged();
}

bool TreeModel::insertRows(int row, int count, const QModelIndex& parent)
{
    if (row < 0 || count != 1)
        return false;

    //    QSqlQuery query = QSqlQuery(db);
    //    int id = query.lastInsertId().toInt();

    beginInsertRows(parent, row, row);

    auto* node_parent = GetNode(parent);
    auto* node_tmp = FindChild(node_parent, row);

    auto* node_new = new Node(0, "New Node", "", node_parent);

    if (node_tmp->mark == 'z') {
        node_parent->left_child = node_new;
        node_new->previous = node_parent;
        node_new->mark = 'z';

        node_tmp->previous = node_new;
        node_new->right_sibling = node_tmp;
        node_tmp->mark = '\0';
    } else {
        node_tmp->previous->right_sibling = node_new;
        node_new->previous = node_tmp->previous;

        node_new->right_sibling = node_tmp;
        node_tmp->previous = node_new;
    }

    endInsertRows();

    InsertRecord(node_parent->id, "New Node");
    return true;
}

bool TreeModel::InsertRecord(const int id_parent, const QString& name)
{

    QSqlQuery query = QSqlQuery(db);

    query.prepare(QString("INSERT INTO %1 (name) VALUES (:name)").arg(table_info.node));
    query.bindValue(":name", name);

    if (!query.exec()) {
        qWarning() << "Failed to add node" << query.lastError().text();
        return false;
    }

    int id = query.lastInsertId().toInt();

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
    auto* node_tmp = FindChild(node_parent, row);
    int id = node_tmp->id;

    beginRemoveRows(parent, row, row);

    if (node_tmp->left_child != nullptr) {
        auto* node_tmp2 = node_tmp->right_sibling;

        if (node_tmp2 == nullptr) {
            node_tmp2 = node_tmp;
        } else {
            while (node_tmp2->right_sibling != nullptr) {
                node_tmp2 = node_tmp2->right_sibling;
            }
        }

        node_tmp->left_child->mark = '\0';
        node_tmp2->right_sibling = node_tmp->left_child;
        node_tmp->left_child->previous = node_tmp2;

        node_tmp->left_child = nullptr;
    }

    if (node_tmp->mark != 'z') {
        if (node_tmp->right_sibling == nullptr) {
            node_tmp->previous->right_sibling = nullptr;
        } else {
            node_tmp->previous->right_sibling = node_tmp->right_sibling;
            node_tmp->right_sibling->previous = node_tmp->previous;
        }
    } else {
        if (node_tmp->right_sibling == nullptr) {
            node_tmp->previous->left_child = nullptr;
        } else {
            node_tmp->previous->left_child = node_tmp->right_sibling;
            node_tmp->right_sibling->previous = node_tmp->previous;
            node_tmp->right_sibling->mark = 'z';
        }
    }

    delete node_tmp;
    node_tmp = nullptr;

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

bool TreeModel::DragRecord()
{
    return true;
}

Qt::ItemFlags TreeModel::flags(const QModelIndex& index) const
{
    auto default_flags = QAbstractItemModel::flags(index);

    if (index.isValid()) {
        if (index.column() == 0 || index.column() == 1)
            return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | default_flags;
        else
            return Qt::ItemIsEditable | default_flags;
    } else
        return default_flags;
}

// Qt::DropActions TreeModel::supportedDropActions() const
//{
//     return Qt::CopyAction | Qt::MoveAction;
// }

QStringList TreeModel::mimeTypes() const
{
    QStringList types;
    types << "application/vnd.text.list";
    return types;
}

QMimeData* TreeModel::mimeData(const QModelIndexList& indexes) const
{
    QMimeData* mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    for (const QModelIndex& index : indexes) {
        if (index.isValid()) {
            Node* node = GetNode(index);
            stream << node->id;
        }
    }

    mimeData->setData("application/vnd.text.list", encodedData);
    return mimeData;
}

// bool TreeModel::IsDescendantOf(Node* possibleDescendant, Node* possibleAncestor)
//{
//     if (!possibleDescendant || !possibleAncestor) {
//         return false;
//     }

//    Node* currentNode = possibleDescendant->previous;
//    while (currentNode) {
//        if (currentNode == possibleAncestor) {
//            return true;
//        }
//        currentNode = currentNode->previous;
//    }
//    return false;
//}

bool TreeModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row,
    int column, const QModelIndex& parent)
{
    if (action == Qt::IgnoreAction)
        return true;

    if (!data->hasFormat("application/vnd.text.list"))
        return false;

    if (column != 0)
        return false;

    QByteArray encodedData = data->data("application/vnd.text.list");
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    QList<int> newItems;
    int newItem;

    while (!stream.atEnd()) {
        stream >> newItem;
        newItems << newItem;
    }
#if 0
    Node* node_parent = GetNode(parent);
    int beginRow = row == -1 ? node_parent->children.count() : row;

    Node* moved_node;

    for (int itemId : newItems) {
        moved_node = FindNode(itemId, node_root);
        if (moved_node) {
            // 如果目标父节点与移动节点的当前父节点相同，跳过此节点
            if (moved_node->parent == node_parent || IsDescendantOf(node_parent, moved_node)) {
                continue;
            }
            QModelIndex movedIndex = createIndex(
                moved_node->parent->children.indexOf(moved_node), 0, moved_node);
            beginRemoveRows(movedIndex.parent(), movedIndex.row(), movedIndex.row());
            moved_node->parent->children.removeOne(moved_node);
            endRemoveRows();

            beginInsertRows(parent, beginRow, beginRow);
            node_parent->children.insert(beginRow, moved_node);
            moved_node->parent = node_parent;
            endInsertRows();
        }
    }
#endif

    return true;
}
