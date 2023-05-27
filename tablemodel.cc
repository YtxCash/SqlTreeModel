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

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0:

            break;
        case 1:

            break;
        case 2:

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

    switch (index.column()) {
    case 0:
        break;
    case 1:
        break;
    case 2:

        return true;
    }

    return false;
}

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{

}

void TableModel::sort(int column, Qt::SortOrder order)
{

}

bool TableModel::insertRows(int row, int count, const QModelIndex &parent)
{

}

bool TableModel::removeRows(int row, int count, const QModelIndex &parent)
{

}

Qt::ItemFlags TableModel::flags(const QModelIndex &index) const
{

}
