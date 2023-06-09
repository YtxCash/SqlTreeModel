﻿#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "tablemodel.h"
#include "treemodel.h"
#include "ui_mainwindow.h"
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnDelete_clicked();

    void on_btnInsert_clicked();

    void on_treeView_clicked(const QModelIndex& index);

private slots:
    void CurrentChanged(const QModelIndex& index);

    void on_btnAppend_clicked();

    void on_treeView_doubleClicked(const QModelIndex& index);

private:
    Ui::MainWindow* ui;

    TreeModel* financial_tree_model;

    QSqlDatabase db;
};
#endif // MAINWINDOW_H
