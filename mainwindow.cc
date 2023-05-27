﻿#include "mainwindow.h"
#include "QtSql/qsqlerror.h"
#include "ui_mainwindow.h"
#include <QInputDialog>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //    db = QSqlDatabase::addDatabase("QSQLITE");
    //    db.setDatabaseName(table_info->database);

    //    if (!db.open()) {
    //        qWarning() << "Failed to open database:" << db.lastError().text();
    //        return;
    //    }

    //    if (!db.isOpen()) {
    //        qWarning() << "Database is not open";
    //        return;
    //    }

    tree_info = new TreeInfo("test.db", "financial", "financial_path");
    tree_model = new TreeModel(*tree_info, this);

    ui->treeView->setModel(tree_model);
    ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->treeView->setDragEnabled(true);
    ui->treeView->setAcceptDrops(true);
    ui->treeView->setDropIndicatorShown(true);
    ui->treeView->setSortingEnabled(true);
    //    ui->treeView->setColumnHidden(1, true);
    ui->treeView->setColumnWidth(0, 200);
    ui->treeView->setExpandsOnDoubleClick(true);
    ui->treeView->header()->setStretchLastSection(true);

    table_info = new TableInfo("test.db", "financial_transaction");
    table_model = new TableModel(*table_info, this);

    ui->tableView->setModel(table_model);
    ui->tableView->setSortingEnabled(true);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);

    connect(ui->treeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &MainWindow::CurrentChanged);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete tree_info;
    delete tree_model;
    delete table_info;
    delete table_model;

    //    db.close();
}

void MainWindow::on_btnDelete_clicked()
{
    auto index = ui->treeView->selectionModel()->selectedRows();
    for (auto index : index) {
        if (!index.isValid())
            return;

        tree_model->removeRows(ui->treeView->currentIndex().row(), 1, index.parent());
    }
}

void MainWindow::on_btnInsert_clicked()
{
    auto indexes = ui->treeView->selectionModel()->selectedRows();
    for (auto index : indexes) {
        if (!index.isValid())
            return;

        tree_model->insertRows(ui->treeView->currentIndex().row(), 1, index.parent());
        ui->treeView->setCurrentIndex(index);
    }
}

void MainWindow::CurrentChanged(const QModelIndex& index)
{
    //    qDebug() << index.data().toString() << "current changed";
}

void MainWindow::on_treeView_clicked(const QModelIndex& index)
{
    //    qDebug() << index.data().toString() << "selected changed";
}

void MainWindow::on_btnAppend_clicked()
{
    auto indexes = ui->treeView->selectionModel()->selectedRows();

    if (indexes.isEmpty())
        indexes << QModelIndex();

    for (auto index : indexes) {
        tree_model->insertRows(0, 1, index);

        auto index_child = tree_model->index(0, 0, index);
        ui->treeView->setCurrentIndex(index_child);
    }
}
