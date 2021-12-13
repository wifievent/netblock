#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <glog/logging.h>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
                                          ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    Json::Value jv;
    if(AppJson::loadFromFile("netblock.json", jv))
    {
        jv["NetBlock"] >> nb_;
    }

    openCheck = nb_.open();

    setDevInfoFromDatabase();
    setDevTableWidget();
    initDevWidget();
}

MainWindow::~MainWindow()
{
    nb_.close();

    Json::Value jv;
    if(AppJson::loadFromFile("netblock.json", jv))
    jv["NetBlock"] << nb_;
    AppJson::saveToFile("netblock.json", jv);

    delete ui;
}

void MainWindow::processHostDetected(StdHost *host)
{
    qDebug() << QString("%1 %2 %3 %4").arg(QString(host->mac_), QString(host->ip_), host->hostName_, host->nickName_);
    DInfo tmp(*host);
    tmp.isConnect_ = true;

    DInfoList::iterator iter;
    for (iter = dInfoList_.begin(); iter != dInfoList_.end(); ++iter)
    {
        if (iter->mac_ == tmp.mac_)
        {
            break;
        }
    }

    QSqlQuery nbQuery(nb_.nbDB_);
    if (iter != dInfoList_.end())
    {
        iter->isConnect_ = true;
        if (iter->hostName_.isNull() && !tmp.hostName_.isNull())
        {
            iter->hostName_ = tmp.hostName_;
            QMutexLocker ml(&nb_.nbDBLock_);
            nbQuery.prepare("UPDATE host SET host_name = :host_name WHERE host_id = :host_id");
            nbQuery.bindValue(":host_name", iter->hostName_);
            nbQuery.bindValue(":host_id", iter->hostId_);
            nbQuery.exec();
        }
        if (iter->nickName_.isNull())
        {
            iter->nickName_ = iter->hostName_;
            QMutexLocker ml(&nb_.nbDBLock_);
            nbQuery.prepare("UPDATE host SET nick_name = :nick_name WHERE host_id = :host_id");
            nbQuery.bindValue(":nick_name", iter->nickName_);
            nbQuery.bindValue(":host_id", iter->hostId_);
            nbQuery.exec();
        }
        if (tmp.ip_ != iter->ip_)
        {
            QMutexLocker ml(&nb_.nbDBLock_);
            nbQuery.prepare("UPDATE host SET last_ip=:last_ip WHERE host_id = :host_id");
            nbQuery.bindValue(":last_ip", QString(tmp.ip_));
            nbQuery.bindValue(":host_id", iter->hostId_);
            nbQuery.exec();
            iter->ip_ = tmp.ip_;
        }
        setDevTableItem();
    }
    else
    {
        QSqlQuery ouiQuery(nb_.ouiDB_);

        {
            QMutexLocker ml(&nb_.ouiDBLock_);
            ouiQuery.prepare("SELECT organ FROM oui WHERE substr(mac, 1, 8) = substr(:mac, 1, 8)");
            ouiQuery.bindValue(":mac", QString(tmp.mac_));
            ouiQuery.exec();

            while (ouiQuery.next())
            {
                tmp.oui_ = ouiQuery.value(0).toString();
            }
        }

        tmp.nickName_ = tmp.hostName_;

        {
            QMutexLocker ml(&nb_.nbDBLock_);
            nbQuery.prepare("INSERT INTO host(mac, last_ip, host_name, nick_name, oui) VALUES(:mac, :last_ip, :host_name, :nick_name, :oui)");
            nbQuery.bindValue(":mac", QString(tmp.mac_));
            nbQuery.bindValue(":last_ip", QString(tmp.ip_));
            nbQuery.bindValue(":host_name", tmp.hostName_);
            nbQuery.bindValue(":nick_name", tmp.nickName_);
            nbQuery.bindValue(":oui", tmp.oui_);
            nbQuery.exec();
        }

        {
            QMutexLocker ml(&nb_.nbDBLock_);
            nbQuery.prepare("SELECT host_id FROM host WHERE mac=:mac");
            nbQuery.bindValue(":mac", QString(tmp.mac_));
            nbQuery.exec();

            nbQuery.next();
            tmp.hostId_ = nbQuery.value(0).toInt();
        }

        dInfoList_.append(tmp);
        if (dInfoList_.length() <= 1)
        {
            setDevTableWidget();
        }
        else
        {
            setDevTableItem();
        }
        resetHostFilter();
    }
}

void MainWindow::processHostDeleted(Host *host)
{
    qDebug() << QString("%1 %2 %3 %4").arg(QString(host->mac_), QString(host->ip_), host->hostName_, host->nickName_);
    DInfo tmp(*host);
    for (DInfoList::iterator iter = dInfoList_.begin(); iter != dInfoList_.end(); ++iter)
    {
        if (tmp.mac_ == iter->mac_)
        {
            iter->isConnect_ = false;
            break;
        }
    }
    setDevTableItem();
}

void MainWindow::setDevInfoFromDatabase()
{
    std::list<DataList> dl = nb_.nbConnect_->selectQuery("SELECT * FROM host ORDER BY last_ip");

    if(dl.size() > 0)
    {
        for(DataList data: dl)
        {
            StdDInfo tmp;
            tmp.hostId_ = std::stoi(data.argv_[0]);
            tmp.mac_ = Mac(data.argv_[1]);
            tmp.ip_ = Ip(data.argv_[2]);
            tmp.hostName_ = data.argv_[3];
            tmp.nickName_ = data.argv_[4];
            tmp.oui_ = data.argv_[5];

            dInfoList_.push_back(tmp);
        }
    }
}

void MainWindow::setDevTableWidget()
{

    setDevTableItem();

    // Table can't be modified
    ui->devTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->devTable->setFocusPolicy(Qt::NoFocus);
    ui->devTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->devTable->resizeColumnToContents(0);
    ui->devTable->horizontalHeader()->setStretchLastSection(true);
}

void MainWindow::setDevTableItem()
{
    ui->devTable->setRowCount(dInfoList_.size());

    int i = 0;
    for(StdDInfoList::iterator iter = dInfoList_.begin(); iter != dInfoList_.end(); ++iter, ++i)
    {
        if(iter->isConnect_)
        {
            QPushButton *btn = new QPushButton();
            btn->setParent(ui->devTable);
            btn->setStyleSheet("QPushButton { margin: 4px; background-color: blue; width: 20px; border-color: black; border-width: 1px; border-radius: 10px; }");
            ui->devTable->setCellWidget(i, 0, btn);
        }
        else
        {
            QPushButton *btn = new QPushButton();
            btn->setParent(ui->devTable);
            btn->setStyleSheet("QPushButton { margin: 4px; background-color: gray; width: 20px; border-color: black; border-width: 1px; border-radius: 10px; }");
            ui->devTable->setCellWidget(i, 0, btn);
        }
        ui->devTable->setItem(i, 1, new QTableWidgetItem(std::string(iter->ip_).data()));
        ui->devTable->setItem(i, 2, new QTableWidgetItem(iter->defaultName().data()));
    }
    resetHostFilter();
}

void MainWindow::initDevWidget()
{
    ui->devInfo->clear();
    ui->devPolicyBtn->setEnabled(false);
    ui->devDeleteBtn->setEnabled(false);
}

void MainWindow::activateBtn()
{
    ui->devPolicyBtn->setEnabled(true);
    ui->devDeleteBtn->setEnabled(true);
}

void MainWindow::resetHostFilter()
{
    ui->hostFilter->clear();
    for(StdDInfoList::iterator iter = dInfoList_.begin(); iter != dInfoList_.end(); ++iter)
    {
        ui->hostFilter->addItem(iter->defaultName().data(), iter->hostId_);
    }
    setHostFilter();
}

void MainWindow::on_devTable_cellClicked(int row, int column)
{
    (void)column;

    ui->devInfo->clear();
    dInfo_ = &dInfoList_[row];

    setListWidgetItem("OUI");
    setListWidgetItem("MAC");
    setListWidgetItem("IP");
    setListWidgetItem("Host_Name");
    QWidget *n_widget = new QWidget();
    QHBoxLayout *n_layout = new QHBoxLayout();
    QLabel *n_label = new QLabel("Nick_Name\t");
    QLineEdit *n_edit = new QLineEdit();
    QPushButton *n_btn = new QPushButton("Edit");
    n_edit->setText(dInfo_->nickName_.data());
    n_layout->addWidget(n_label);
    n_layout->addWidget(n_edit);
    n_layout->addWidget(n_btn);
    n_layout->addStretch();
    n_widget->setLayout(n_layout);
    QListWidgetItem *n_item = new QListWidgetItem();
    n_item->setSizeHint(n_widget->sizeHint());
    ui->devInfo->addItem(n_item);
    ui->devInfo->setItemWidget(n_item, n_widget);
    lineEdit_ = n_edit;
    connect(n_btn, SIGNAL(clicked()), this, SLOT(onEditBtnClicked()));
    activateBtn();
}

void MainWindow::setListWidgetItem(QString str)
{
    QWidget *widget = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout();
    QLabel *key_label = new QLabel(str + "\t");
    QLabel *val_label = new QLabel();
    if (str == "OUI")
    {
        val_label->setText(dInfo_->oui_.data());
    }
    else if (str == "MAC")
    {
        val_label->setText(std::string(dInfo_->mac_).data());
    }
    else if (str == "IP")
    {
        val_label->setText(std::string(dInfo_->ip_).data());
    }
    else if (str == "Host_Name")
    {
        val_label->setText(dInfo_->hostName_.data());
    }
    layout->addWidget(key_label);
    layout->addWidget(val_label);
    layout->addStretch();
    widget->setLayout(layout);
    QListWidgetItem *item = new QListWidgetItem();
    item->setSizeHint(widget->sizeHint());
    ui->devInfo->addItem(item);
    ui->devInfo->setItemWidget(item, widget);
}

void MainWindow::on_devPolicyBtn_clicked()
{
    DLOG(INFO) << "Click PolicyBtn";
    selectedHostId_ = dInfo_->hostId_;
    ui->tabWidget->setCurrentIndex(1);
    setHostFilter();
}

void MainWindow::on_devDeleteBtn_clicked()
{
    std::string query("DELETE FROM host WHERE host_id=:host_id");
    query.replace(query.find(":host_id"), std::string(":host_id").length(), std::to_string(dInfo_->hostId_));
    nb_.nbConnect_->sendQuery(query);

    dInfoList_.erase(std::find(dInfoList_.begin(), dInfoList_.end(), *dInfo_));
    dInfo_ = nullptr;
    setDevTableItem();
    ui->devInfo->clear();
}

void MainWindow::onEditBtnClicked()
{
    dInfo_->nickName_ = lineEdit_->text().toStdString();
    lineEdit_->setText(dInfo_->nickName_.data());

    std::string query("UPDATE host SET nick_name=':nick_name' WHERE host_id=:host_id");
    query.replace(query.find(":nick_name"), std::string(":nick_name").length(), dInfo_->nickName_);
    query.replace(query.find(":host_id"), std::string(":host_id").length(), std::to_string(dInfo_->hostId_));
    nb_.nbConnect_->sendQuery(query);

    DLOG(INFO) << dInfo_->hostId_ << dInfo_->nickName_;

    setDevTableItem();
    resetHostFilter();
}

void MainWindow::setHostFilter()
{
    ui->hostFilter->setCurrentIndex(ui->hostFilter->findData(selectedHostId_));
}

void MainWindow::on_tableWidget_itemSelectionChanged()
{
    QModelIndexList indexList = ui->tableWidget->selectionModel()->selectedIndexes();
    if (indexList.size() > 1 && indexList.constFirst().column() != indexList.constLast().column())
    {
        ui->policyEditButton->setEnabled(true);
    }
    if (!indexList.isEmpty())
    {
        ui->policyDeleteButton->setDisabled(true);
        ui->policyEditButton->setDisabled(true);
        ui->policyAddButton->setEnabled(true);
        QTableWidgetItem *firstItem = ui->tableWidget->item(indexList.constFirst().row(), indexList.constFirst().column());
        if (firstItem != nullptr)
        {
            selectedPolicyId_ = firstItem->data(Qt::UserRole).toInt();
            ui->policyEditButton->setEnabled(true);
            ui->policyDeleteButton->setEnabled(true);
            ui->policyAddButton->setDisabled(true);
            return;
        }
        selectedPolicyId_ = 0;
        return;
    }
    selectedPolicyId_ = 0;
}

void MainWindow::on_hostFilter_currentIndexChanged(int index)
{
    (void)index;

    if (ui->hostFilter->count() == (int)dInfoList_.size())
    {
        selectedHostId_ = ui->hostFilter->currentData().toInt();
        setPolicyListFromDatabase();
        setPolicyTable();
        ui->tableWidget->clearSelection();
        ui->policyAddButton->setDisabled(true);
        ui->policyEditButton->setDisabled(true);
        ui->policyDeleteButton->setDisabled(true);
    }
}

void MainWindow::on_policyAddButton_clicked()
{
    DLOG(INFO) << "policy add button clicked";
    openPolicyConfig();
}

void MainWindow::on_policyEditButton_clicked()
{
    DLOG(INFO) << "policy edit button clicked";
    openPolicyConfig();
}

void MainWindow::on_policyDeleteButton_clicked()
{
    DLOG(INFO) << "policy delete buttton clicked";

    std::string query("DELETE FROM policy WHERE policy_id=:policy_id");
    query.replace(query.find(":policy_id"), std::string(":policy_id").length(), std::to_string(selectedPolicyId_));
    nb_.nbConnect_->sendQuery(query);

    setPolicyListFromDatabase();
    setPolicyTable();
    ui->tableWidget->clearSelection();
    nb_.updateHosts();
    nb_.block();
}

void MainWindow::openPolicyConfig()
{
    selectedHostId_ = ui->hostFilter->currentData().toInt();

    PolicyConfig *policyConfig;
    QModelIndexList indexList = ui->tableWidget->selectionModel()->selectedIndexes();
    policyConfig = new PolicyConfig(indexList, selectedPolicyId_, selectedHostId_, nb_.nbConnect_);

    policyConfig->setModal(true);

    int result = policyConfig->exec();

    if (result == QDialog::Accepted)
    {
        setPolicyListFromDatabase();
        setPolicyTable();
        ui->tableWidget->clearSelection();
        nb_.updateHosts();
        nb_.block();
        DLOG(INFO) << "Policy config Accepted";
    }
}

void MainWindow::setPolicyListFromDatabase()
{
    policyList_.clear();

    std::string query("SELECT policy_id, start_time, end_time, day_of_the_week, host_id FROM policy WHERE host_id = :host_id ORDER BY day_of_the_week ASC");
    query.replace(query.find(":host_id"), std::string(":host_id").length(), std::to_string(selectedHostId_));
    std::list<DataList> dl = nb_.nbConnect_->selectQuery(query);

    if(dl.size() > 0)
    {
        for(DataList data: dl)
        {
            std::vector<std::string> tmp;
            tmp.push_back(data.argv_[0]);
            tmp.push_back(data.argv_[1]);
            tmp.push_back(data.argv_[2]);
            tmp.push_back(data.argv_[3]);
            tmp.push_back(data.argv_[4]);

            PolicyObj tmpObj;
            tmpObj.set(tmp);

            policyList_.push_back(tmpObj);
        }
    }
}

void MainWindow::resetPolicyTable()
{
    for (int i = 0; i < ui->tableWidget->rowCount(); ++i)
    {
        for (int j = 0; j < ui->tableWidget->columnCount(); ++j)
        {
            ui->tableWidget->setItem(i, j, nullptr);
            ui->tableWidget->setColumnWidth(j, 100);
            ui->tableWidget->clearSpans();
        }
    }
}

void MainWindow::setItemPolicy(int row, int column, int policyId, int span)
{
    if (span > 1)
    {
        ui->tableWidget->setSpan(row, column, span, 1);
    }

    QTableWidgetItem *itm = new QTableWidgetItem();
    itm->setBackground(Qt::gray);
    ui->tableWidget->setItem(row, column, itm);
    itm->setData(Qt::UserRole, policyId);
}

void MainWindow::setPolicyTable()
{
    resetPolicyTable();

    for(std::vector<PolicyObj>::iterator iter = policyList_.begin(); iter != policyList_.end(); ++iter)
    {
        int startHour = std::stoi(iter->getStartTime().substr(0, 2));
        int startMin = std::stoi(iter->getStartTime().substr(2));
        int endHour = std::stoi(iter->getEndTime().substr(0, 2));
        int endMin = std::stoi(iter->getEndTime().substr(2));

        int startRow = startHour + startMin / MINUTE::HOUR;
        int endRow = endHour + endMin / MINUTE::HOUR;
        int column = iter->getDayOfTheWeek();
        int span = 0;

        if(iter->getStartTime() <= iter->getEndTime())
        {
            span = endRow - startRow;
            setItemPolicy(startRow, column, iter->getPolicyId(), span);
        }
        else
        {
            span = 24 - startRow;
            setItemPolicy(startRow, column, iter->getPolicyId(), span);

            span = endRow;
            setItemPolicy(0, column, iter->getPolicyId(), span);
        }
    }
}

void MainWindow::load(Json::Value& jv)
{
    jo["rect"] >> GJson::rect(this);
    jo["tableWidget"] >> GJson::columnSizes(ui->devTable);
}

void MainWindow::save(Json::Value& jv)
{
    jo["rect"] << GJson::rect(this);
    jo["tableWidget"] << GJson::columnSizes(ui->devTable);
}
