#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QInputDialog>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    table_info = new TableInfo("test.db", "node", "node_path");

    tree_model = new TreeModel(*table_info, this);

    ui->treeView->setModel(tree_model);

    ui->treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->treeView->setDragEnabled(true);
    ui->treeView->setAcceptDrops(true);
    ui->treeView->setDropIndicatorShown(true);
    ui->treeView->setSortingEnabled(true);

    //    ui->treeView->setColumnHidden(1, true);
    // 如果有需要，设置列宽
    ui->treeView->setColumnWidth(0, 400);
    ui->treeView->setColumnWidth(1, 300);

    ui->treeView->setExpandsOnDoubleClick(true);
    ui->treeView->header()->setStretchLastSection(true);

    connect(ui->treeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &MainWindow::CurrentChanged);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete table_info;
    delete tree_model;
}

void MainWindow::on_btnDelete_clicked()
{
    // 获取所选节点的索引
    auto selected_index = ui->treeView->currentIndex();

    if (!selected_index.isValid())
        return;

    tree_model->removeRows(ui->treeView->currentIndex().row(), 1, selected_index.parent());
}

void MainWindow::on_btnInsert_clicked()
{
    auto selected_index = ui->treeView->currentIndex();
    if (!selected_index.isValid())
        return;

    tree_model->insertRows(ui->treeView->currentIndex().row(), 1, selected_index.parent());
}

void MainWindow::CurrentChanged(const QModelIndex& index)
{
    qDebug() << index.data().toString() << "current changed";
}

void MainWindow::on_treeView_clicked(const QModelIndex& index)
{
    qDebug() << index.data().toString() << "click";
}

void MainWindow::on_btnAppend_clicked()
{
    auto selected_index = ui->treeView->currentIndex();
    if (!selected_index.isValid())
        selected_index = QModelIndex();

    tree_model->insertRows(0, 1, selected_index);
}
