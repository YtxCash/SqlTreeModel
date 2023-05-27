#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QAbstractTableModel>
#include <QSqlDatabase>

struct Transaction {
    int id { 0 };
    QString note { "" };
    QString description { "" };

    Transaction(int id, QString note, QString description)
        : id { id }
        , note { note }
        , description { description }
    {
    }
};

struct TableInfo {
    QString transaction { "" };

    TableInfo(QString transaction)
        : transaction { transaction }
    {
    }
};

class TableModel : public QAbstractTableModel {

public:
    explicit TableModel(const QSqlDatabase& db, const TableInfo& table_info, QObject* parent = nullptr);
    ~TableModel();

public:
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index,
        int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value,
        int role = Qt::EditRole) override;

    QVariant headerData(int section, Qt::Orientation orientation,
        int role = Qt::DisplayRole) const override;

    void sort(int column, Qt::SortOrder order) override;

    bool insertRows(int row, int count,
        const QModelIndex& parent = QModelIndex()) override;
    bool removeRows(int row, int count,
        const QModelIndex& parent = QModelIndex()) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

private:
    void ConstructTable(const QSqlDatabase& db);

private:
    QList<Transaction*> transactions;
    QSqlDatabase db;
    TableInfo table_info;

    QStringList headers;
};

#endif // TABLEMODEL_H
