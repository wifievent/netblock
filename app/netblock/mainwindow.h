#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QLineEdit>
#include <QTableWidgetItem>

#include "netblock.h"
#include "dinfo.h"
#include "policyobj.h"
#include "policyconfig.h"

#include "appjson.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow, StateObj
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool openCheck;

public slots:
    void processHostDetected(StdHost* host);

    void processHostDeleted(StdHost* host);

private slots:
    void on_devTable_cellClicked(int row, int column);

    void on_devPolicyBtn_clicked();

    void on_devDeleteBtn_clicked();

    void onEditBtnClicked();

private:
    Ui::MainWindow *ui;

    NetBlock nb_;

    DInfo* dInfo_;
    DInfoList dInfoList_;
    QLineEdit *lineEdit_;

    void setDevInfoFromDatabase();
    void setDevTableWidget();
    void setDevTableItem();
    void initDevWidget();
    void activateBtn();
    void setListWidgetItem(QString str);
    void resetHostFilter();

private slots:
    void on_tableWidget_itemSelectionChanged();

    void on_hostFilter_currentIndexChanged(int index);

    void on_policyAddButton_clicked();

    void on_policyEditButton_clicked();

    void on_policyDeleteButton_clicked();

private:
    int selectedHostId_ = 0;
    int selectedPolicyId_ = 0;
    QVector<PolicyObj> policyList_;

    enum MINUTE {
        HOUR=60,
    };

    void setHostFilter();
    void openPolicyConfig();
    void setPolicyListFromDatabase();
    void resetPolicyTable();
    void setItemPolicy(int row, int column, int policyId, int span);
    void setPolicyTable();

    void load(Json::Value& json) override;
    void save(Json::Value& json) override;
};

#endif // MAINWINDOW_H
