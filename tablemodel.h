#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include "tableinfo.h"
#include <QAbstractTableModel>
#include <QSqlDatabase>

struct Entry {
    int id { 0 };
    QString name { "" };
    QString description { "" };

    Entry(int id, QString name, QString description)
        : id(id)
        , name(name)
        , description(description)
    {
    }
};

class TableModel : public QAbstractTableModel {

public:
    explicit TableModel(const TableInfo& table_info, QObject* parent = nullptr);
    ~TableModel();

private:
    QList<Entry> entries;
    QSqlDatabase db;
    TableInfo table_info;

    QStringList headers;
};

#endif // TABLEMODEL_H
