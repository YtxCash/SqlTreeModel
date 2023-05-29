#include "mainwindow.h"
#include "QtSql/qsqlerror.h"
#include "comboboxdelegate.h"
#include "ui_mainwindow.h"
#include <QCompleter>
#include <QInputDialog>
#include <QTableView>

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

    auto financial_tree_info = TreeInfo("financial", "financial_path");
    financial_tree_model = new TreeModel(db, financial_tree_info, ui->treeView);

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

    ui->tabWidget->setMovable(true);
    ui->tabWidget->setTabsClosable(true);
    ui->tabWidget->setElideMode(Qt::ElideNone);
    ui->gridLayout_2->setContentsMargins(0, 0, 0, 0);

    connect(ui->treeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &MainWindow::CurrentChanged);
}

MainWindow::~MainWindow()
{
    delete ui;
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

void MainWindow::on_treeView_doubleClicked(const QModelIndex& index)
{
    auto* node = static_cast<Node*>(index.internalPointer());

    if (node->children.isEmpty()) {
        auto* table_view = new QTableView();
        auto table_info = TableInfo("financial_transaction", node->id);
        auto* table_model = new TableModel(db, table_info, table_view);
        auto* table_delegate = new ComboBoxDelegate(financial_tree_model->GetLeafPaths(), table_model);
        connect(financial_tree_model, &TreeModel::LeafPaths, table_delegate, &ComboBoxDelegate::ReceiveLeafPaths);

        table_view->setItemDelegateForColumn(2, table_delegate);
        table_view->setModel(table_model);
        table_view->setSortingEnabled(true);
        table_view->horizontalHeader()->setStretchLastSection(true);
        table_view->setSelectionMode(QAbstractItemView::SingleSelection);
        table_view->setSelectionBehavior(QAbstractItemView::SelectRows);

        ui->tabWidget->addTab(table_view, node->name);
    }
}
