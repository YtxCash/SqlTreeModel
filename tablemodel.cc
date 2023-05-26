#include "tablemodel.h"
#include <QSqlError>

TableModel::TableModel(const TableInfo& table_info, QObject* parent)
    : QAbstractTableModel { parent }
    , table_info { table_info }
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
}

TableModel::~TableModel()
{
    db.close();
}

int TableModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return transactions.size();
}

int TableModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return headers.size();
}

QVariant TableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    auto transaction = static_cast<Transaction>(index.internalPointer());

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

bool TableModel::setData(const QModelIndex& index, const QVariant& value, int role)
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
