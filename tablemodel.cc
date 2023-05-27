#include "tablemodel.h"
#include <QSqlError>

TableModel::TableModel(const QSqlDatabase& db, const TableInfo& table_info, QObject* parent)
    : QAbstractTableModel { parent }
    , table_info { table_info }
    , db { db }
{

    headers << "ID"
            << "Note"
            << "Description";
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
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return headers.at(section);

    return QVariant();
}

void TableModel::sort(int column, Qt::SortOrder order)
{
    emit layoutAboutToBeChanged();

    auto Compare = [column, order](const Transaction& lhs, const Transaction& rhs) -> bool {
        bool result;
        switch (column) {
        case 0:
            result = lhs.id < rhs.id;
            break;
        case 1:
            result = lhs.note < rhs.note;
            break;
        case 2:
            result = lhs.description < rhs.description;
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
}

bool TableModel::removeRows(int row, int count, const QModelIndex& parent)
{
}

Qt::ItemFlags TableModel::flags(const QModelIndex& index) const
{
}
