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
