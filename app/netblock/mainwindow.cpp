#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setDevInfo() {

}
void MainWindow::setDevTableWidget() {

}
void MainWindow::initDevListWidget() {

}
void MainWindow::setListWidgetItem(QString) {

}
void MainWindow::activateBtn() {

}
void MainWindow::clearDevices() {

}
void MainWindow::viewDevState() {
    
}