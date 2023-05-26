#ifndef TABLEINFO_H
#define TABLEINFO_H
#include <QString>

struct TableInfo {
    QString database { "" };
    QString node { "" };
    QString node_path { "" };

    TableInfo(QString database, QString node, QString node_path)
        : database(database)
        , node(node)
        , node_path(node_path)
    {
    }
};

#endif // TABLEINFO_H
