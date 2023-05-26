#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QAbstractTableModel>
#include <QSqlDatabase>

class TableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit TableModel(QObject* parent = nullptr);

private:
private:
};

#endif // TABLEMODEL_H
