#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QAbstractTableModel>
#include <QSqlDatabase>

struct Transaction {
    int id { 0 };
    int source { 0 };
    QString note { "" };
    QString description { "" };
    int target { 0 };
    double debit { 0.00 };
    double credit { 0.00 };

    Transaction(int id, int source, int target)
        : id { id }
        , source { source }
        , target { target }
    {
    }
};

struct TableInfo {
    QString transaction { "" };
    int id_selected { 0 };

    TableInfo(QString transaction, int id)
        : transaction { transaction }
        , id_selected { id }
    {
    }
};

class TableModel : public QAbstractTableModel {
    Q_OBJECT

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
    void ConstructTable(const QSqlDatabase& db, int id);
    bool InsertRecord();
    bool UpdateRecord(int id, QString column, QString string);
    bool DeleteRecord(int id);

private:
    QList<Transaction*> transactions;
    QSqlDatabase db;
    TableInfo table_info;

    int id_last_insert;
    QStringList headers;
};

#endif // TABLEMODEL_H
