#include "mainwindow.h"
#include "QtSql/qsqlerror.h"
#include "comboboxdelegate.h"
#include "ui_mainwindow.h"
#include <QInputDialog>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("test.db");

    if (!db.open()) {
        qWarning() << "Failed to open database:" << db.lastError().text();
        return;
    }

    if (!db.isOpen()) {
        qWarning() << "Database is not open";
        return;
    }

    ui->setupUi(this);

    financial_tree_info = new TreeInfo("financial", "financial_path");
    financial_tree_model = new TreeModel(db, *financial_tree_info, this);

    ui->treeView->setModel(financial_tree_model);
    ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->treeView->setDragEnabled(true);
    ui->treeView->setAcceptDrops(true);
    ui->treeView->setDropIndicatorShown(true);
    ui->treeView->setSortingEnabled(true);
    //    ui->treeView->setColumnHidden(1, true);
    ui->treeView->setColumnWidth(0, 200);
    ui->treeView->setExpandsOnDoubleClick(true);
    ui->treeView->header()->setStretchLastSection(true);

    financial_table_info = new TableInfo("financial_transaction");
    financial_table_model = new TableModel(db, *financial_table_info, this);

    auto delegate = new ComboBoxDelegate(financial_tree_model->leaf_paths, this);
    ui->tableView->setItemDelegateForColumn(2, delegate);

    ui->tableView->setModel(financial_table_model);
    ui->tableView->setSortingEnabled(true);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);

    connect(ui->treeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &MainWindow::CurrentChanged);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete financial_tree_info;
    delete financial_tree_model;
    delete financial_table_info;
    delete financial_table_model;

    db.close();
}

void MainWindow::on_btnDelete_clicked()
{
    auto index = ui->treeView->selectionModel()->selectedRows();
    for (auto index : index) {
        if (!index.isValid())
            return;

        financial_tree_model->removeRows(ui->treeView->currentIndex().row(), 1, index.parent());
    }
}

void MainWindow::on_btnInsert_clicked()
{
    auto indexes = ui->treeView->selectionModel()->selectedRows();
    for (auto index : indexes) {
        if (!index.isValid())
            return;

        financial_tree_model->insertRows(ui->treeView->currentIndex().row(), 1, index.parent());
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
        financial_tree_model->insertRows(0, 1, index);

        auto index_child = financial_tree_model->index(0, 0, index);
        ui->treeView->setCurrentIndex(index_child);
    }
}
