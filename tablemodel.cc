#include "tablemodel.h"
#include <QSqlError>
#include <QSqlQuery>

TableModel::TableModel(const QSqlDatabase& db, const TableInfo& table_info, QObject* parent)
    : QAbstractTableModel { parent }
    , table_info { table_info }
    , db { db }
{

    headers << "ID"
            << "Source"
            << "Note"
            << "Description"
            << "Target"
            << "Debit"
            << "Credit";

    ConstructTable(db, table_info.id_selected);
}

TableModel::~TableModel()
{
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

    int row = index.row();

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0:
            return transactions.at(row)->id;
        case 1:
            return transactions.at(row)->note;
        case 2:
            return transactions.at(row)->description;
        default:
            return QVariant();
        }
    }

    return QVariant();
}

bool TableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    int row = index.row();

    switch (index.column()) {
    case 0:
        transactions[row]->id = value.toInt();
        return true;
    case 1:
        transactions[row]->note = value.toString();
        return true;
    case 2:
        transactions[row]->description = value.toString();
        return true;
    }

    return false;
}

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return headers.at(section);

    return QVariant();
}

void TableModel::sort(int column, Qt::SortOrder order)
{
    emit layoutAboutToBeChanged();

    auto Compare = [column, order](const Transaction* lhs, const Transaction* rhs) -> bool {
        bool result;
        switch (column) {
        case 0:
            result = lhs->id < rhs->id;
            break;
        case 1:
            result = lhs->note < rhs->note;
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

    std::sort(transactions.begin(), transactions.end(), Compare);

    emit layoutChanged();
}

bool TableModel::insertRows(int row, int count, const QModelIndex& parent)
{
    if (row < 0 || count != 1 || !parent.isValid())
        return false;

    //    auto transaction = new Transaction(0, "", "");
    //    transactions.insert(row, transaction);

    return true;
}

bool TableModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (row < 0 || count != 1 || !parent.isValid())
        return false;

    transactions.remove(row, 1);

    return true;
}

Qt::ItemFlags TableModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    auto default_flags = QAbstractItemModel::flags(index);

    switch (index.column()) {
    case 0:
        return default_flags;
    default:
        return Qt::ItemIsEditable | default_flags;
    }
}

void TableModel::ConstructTable(const QSqlDatabase& db, int id_selected)
{
    auto query = QSqlQuery(db);

    query.prepare(QString("SELECT :id_selected, note, description FROM %1").arg(table_info.transaction));
    query.bindValue(":id_selected", id_selected);

    if (!query.exec()) {
        qWarning() << QString("Error query data from %1").arg(table_info.transaction)
                   << query.lastError().text();
    }

    int id = 0;
    QString note;
    QString description;

    while (query.next()) {
        id = query.value(0).toInt();
        note = query.value(1).toString();
        description = query.value(2).toString();

        //        auto* transaction = new Transaction(id, note, description);
        //        transactions.emplace_back(transaction);
    }
}

bool TableModel::InsertRecord()
{
    auto query = QSqlQuery(db);

    query.prepare(QString("INSERT INTO %1 DEFAULT VALUES ").arg(table_info.transaction));

    if (!query.exec()) {
        qWarning() << "Failed to add node" << query.lastError().text();
        return false;
    }

    id_last_insert = query.lastInsertId().toInt();
}

bool TableModel::UpdateRecord(int id, QString column, QString string)
{
}

bool TableModel::DeleteRecord(int id)
{
}
