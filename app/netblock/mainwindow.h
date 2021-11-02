#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QList>

#include "netblock.h"
#include "dinfo.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setDevInfo();
    void setDevTableWidget();
    void initDevListWidget();
    void setListWidgetItem(QString);
    void activateBtn();
    void clearDevices();
    void viewDevState();

signals:
    void sendMac(const QString mac);
    void sendHostSelect(const int hostId);
    void sendReload();

private:
    Ui::MainWindow *ui;

    dInfo dInfo;
    NetBlock nb;
    QVector<dInfo> devices;
};

#endif // MAINWINDOW_H
