#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
	ui->setupUi(this);

    connect(&nb_.lhm_, &LiveHostMgr::hostDetected, this, &MainWindow::processHostDetected, Qt::BlockingQueuedConnection);
    connect(&nb_.lhm_, &LiveHostMgr::hostDeleted, this, &MainWindow::processHostDeleted, Qt::BlockingQueuedConnection);

    QJsonObject jo = GJson::loadFromFile();
    jo["MainWindow"] >> *this;
    jo["NetBlock"] >> nb_;

    GThreadMgr::suspendStart();

    openCheck = nb_.open();

    GThreadMgr::resumeStart();

    setDevInfoFromDatabase();
    setDevTableWidget();
    initDevWidget();
}

MainWindow::~MainWindow()
{
    nb_.close();

	QJsonObject jo = GJson::loadFromFile();
	jo["MainWindow"] << *this;
	jo["NetBlock"] << nb_;
	GJson::saveToFile(jo);

    delete ui;
}

void MainWindow::processHostDetected(Host* host) {
    qDebug() << QString("host detected %1 %2 %3 %4").arg(QString(host->mac_), QString(host->ip_), host->hostName_, host->nickName_);
    DInfo tmp(*host);
    tmp.isConnect_ = true;

    DInfoList::iterator iter;
    for(iter = dInfoList_.begin(); iter != dInfoList_.end(); ++iter) {
        if(iter->mac_ == tmp.mac_) {
            break;
        }
    }

    QSqlQuery nbQuery(nb_.nbDB_);
    if(iter != dInfoList_.end()) {
        iter->isConnect_ = true;
        if(iter->hostName_.isNull() && !tmp.hostName_.isNull()) {
            iter->hostName_ = tmp.hostName_;
            QMutexLocker ml(&nb_.nbDBLock_);
            nbQuery.prepare("UPDATE host SET host_name = :host_name WHERE host_id = :host_id");
            nbQuery.bindValue(":host_name", iter->hostName_);
            nbQuery.bindValue(":host_id", iter->hostId_);
            nbQuery.exec();
        }
        if(iter->nickName_.isNull()) {
            iter->nickName_ = iter->hostName_;
            QMutexLocker ml(&nb_.nbDBLock_);
            nbQuery.prepare("UPDATE host SET nick_name = :nick_name WHERE host_id = :host_id");
            nbQuery.bindValue(":nick_name", iter->nickName_);
            nbQuery.bindValue(":host_id", iter->hostId_);
        }
        if(tmp.ip_ != iter->ip_) {
            QMutexLocker ml(&nb_.nbDBLock_);
            nbQuery.prepare("UPDATE host SET last_ip=:last_ip WHERE host_id = :host_id");
            nbQuery.bindValue(":last_ip", QString(tmp.ip_));
            nbQuery.bindValue(":host_id", iter->hostId_);
            nbQuery.exec();
            iter->ip_ = tmp.ip_;
        }
        setDevTableItem();
    } else {
        QSqlQuery ouiQuery(nb_.ouiDB_);

        {
            QMutexLocker ml(&nb_.ouiDBLock_);
            ouiQuery.prepare("SELECT organ FROM oui WHERE substr(mac, 1, 8) = substr(:mac, 1, 8)");
            ouiQuery.bindValue(":mac", QString(tmp.mac_));
            ouiQuery.exec();
        }

        while(ouiQuery.next()) {
            tmp.oui_ = ouiQuery.value(0).toString();
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
        }
        nbQuery.next();
        tmp.hostId_ = nbQuery.value(0).toInt();

        dInfoList_.append(tmp);
        if(dInfoList_.length() <= 1) {
            setDevTableWidget();
        }
        else {
            setDevTableItem();
        }
        resetHostFilter();
    }
}

void MainWindow::processHostDeleted(Host* host) {
    qDebug() << QString("host deleted %1 %2 %3 %4").arg(QString(host->mac_), QString(host->ip_), host->hostName_, host->nickName_);
    DInfo tmp(*host);
    for(DInfoList::iterator iter = dInfoList_.begin(); iter != dInfoList_.end(); ++iter) {
        if(tmp.mac_ == iter->mac_) {
            iter->isConnect_ = false;
            break;
        }
    }
    setDevTableItem();
}

void MainWindow::setDevInfoFromDatabase() {
    QSqlQuery nbQuery(nb_.nbDB_);
    {
        QMutexLocker ml(&nb_.nbDBLock_);
        nbQuery.exec("SELECT * FROM host ORDER BY last_ip");
    }

    while(nbQuery.next()) {
        DInfo tmp;
        tmp.hostId_ = nbQuery.value(0).toInt();
        tmp.mac_ = GMac(nbQuery.value(1).toString());
        tmp.ip_ = GIp(nbQuery.value(2).toString());
        tmp.hostName_ = nbQuery.value(3).toString();
        tmp.nickName_ = nbQuery.value(4).toString();
        tmp.oui_ = nbQuery.value(5).toString();

        dInfoList_.append(tmp);
    }

}

void MainWindow::setDevTableWidget() {

    setDevTableItem();

    // Table can't be modified
    ui->devTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->devTable->setFocusPolicy(Qt::NoFocus);
    ui->devTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->devTable->resizeColumnToContents(0);
    ui->devTable->horizontalHeader()->setStretchLastSection(true);
}

void MainWindow::setDevTableItem() {
    ui->devTable->setRowCount(dInfoList_.size());

    int i = 0;
    for(DInfoList::iterator iter = dInfoList_.begin(); iter != dInfoList_.end(); ++iter, ++i) {
        if(iter->isConnect_) {
            QPushButton *btn = new QPushButton();
            btn->setParent(ui->devTable);
            btn->setStyleSheet("QPushButton { margin: 4px; background-color: blue; width: 20px; border-color: black; border-width: 1px; border-radius: 10px; }");
            ui->devTable->setCellWidget(i, 0, btn);
        } else {
            QPushButton *btn = new QPushButton();
            btn->setParent(ui->devTable);
            btn->setStyleSheet("QPushButton { margin: 4px; background-color: gray; width: 20px; border-color: black; border-width: 1px; border-radius: 10px; }");
            ui->devTable->setCellWidget(i, 0, btn);
        }
        ui->devTable->setItem(i, 1, new QTableWidgetItem(QString(iter->ip_)));
        ui->devTable->setItem(i, 2, new QTableWidgetItem(iter->defaultName()));
    }
    resetHostFilter();
}

void MainWindow::initDevWidget() {
    ui->devInfo->clear();
    ui->devPolicyBtn->setEnabled(false);
    ui->devDeleteBtn->setEnabled(false);
}

void MainWindow::activateBtn() {
    ui->devPolicyBtn->setEnabled(true);
    ui->devDeleteBtn->setEnabled(true);
}

void MainWindow::resetHostFilter() {
    ui->hostFilter->clear();
    for(DInfoList::iterator iter = dInfoList_.begin(); iter != dInfoList_.end(); ++iter) {
        ui->hostFilter->addItem(iter->defaultName(), iter->hostId_);
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
    n_edit->setText(dInfo_->nickName_);
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
    if(str == "OUI") {
        val_label->setText(dInfo_->oui_);
    }else if(str == "MAC") {
        val_label->setText(QString(dInfo_->mac_));
    }else if(str == "IP") {
        val_label->setText(QString(dInfo_->ip_));
    }else if(str == "Host_Name") {
        val_label->setText(dInfo_->hostName_);
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
    qDebug() << "Click PolicyBtn";
    selectedHostId_ = dInfo_->hostId_;
    ui->tabWidget->setCurrentIndex(1);
    setHostFilter();
}

void MainWindow::on_devDeleteBtn_clicked()
{
    qDebug() << "test: " << dInfoList_.indexOf(*dInfo_);
    QSqlQuery nbQuery(nb_.nbDB_);

    {
        QMutexLocker ml(&nb_.nbDBLock_);
        nbQuery.prepare("DELETE FROM host WHERE host_id=:host_id");
        nbQuery.bindValue(":host_id", dInfo_->hostId_);
        nbQuery.exec();
    }

    dInfoList_.removeAt(dInfoList_.indexOf(*dInfo_));
    dInfo_ = nullptr;
    setDevTableItem();
    ui->devInfo->clear();
}

void MainWindow::onEditBtnClicked()
{
    dInfo_->nickName_ = lineEdit_->text();
    lineEdit_->setText(dInfo_->nickName_);
    QSqlQuery nbQuery(nb_.nbDB_);

    {
        QMutexLocker ml(&nb_.nbDBLock_);
        nbQuery.prepare("UPDATE host SET nick_name=:nick_name WHERE host_id=:host_id");
        nbQuery.bindValue(":nick_name", dInfo_->nickName_);
        nbQuery.bindValue(":host_id", dInfo_->hostId_);
        nbQuery.exec();
    }

    qDebug() << dInfo_->hostId_ << dInfo_->nickName_;

    setDevTableItem();
    resetHostFilter();
}

void MainWindow::setHostFilter() {
    ui->hostFilter->setCurrentIndex(ui->hostFilter->findData(selectedHostId_));
}

void MainWindow::on_tableWidget_itemSelectionChanged()
{
    QModelIndexList indexList = ui->tableWidget->selectionModel()->selectedIndexes();
    if (indexList.size() > 1 && indexList.constFirst().column() != indexList.constLast().column()) {
        ui->policyEditButton->setEnabled(true);
    }
    if (!indexList.isEmpty()) {
        ui->policyDeleteButton->setDisabled(true);
        ui->policyEditButton->setDisabled(true);
        ui->policyAddButton->setEnabled(true);
        QTableWidgetItem *firstItem = ui->tableWidget->item(indexList.constFirst().row(), indexList.constFirst().column());
        if (firstItem != nullptr) {
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

    if(ui->hostFilter->count() == dInfoList_.length()) {
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
    qDebug() << "policy add buttton clicked";
    openPolicyConfig();
}

void MainWindow::on_policyEditButton_clicked()
{
    qDebug() << "policy edit buttton clicked";
    openPolicyConfig();
}

void MainWindow::on_policyDeleteButton_clicked()
{
    qDebug() << "policy delete buttton clicked";
    QSqlQuery nbQuery(nb_.nbDB_);

    {
        QMutexLocker ml(&nb_.nbDBLock_);
        nbQuery.prepare("DELETE FROM policy WHERE policy_id=:policy_id");
        nbQuery.bindValue(":policy_id", selectedPolicyId_);
        nbQuery.exec();
    }

    setPolicyListFromDatabase();
    setPolicyTable();
    ui->tableWidget->clearSelection();
    nb_.updateHosts();
    nb_.block();
}

void MainWindow::openPolicyConfig() {
    QSqlQuery nbQuery(nb_.nbDB_);

    selectedHostId_ = ui->hostFilter->currentData().toInt();

    PolicyConfig *policyConfig;
    QModelIndexList indexList = ui->tableWidget->selectionModel()->selectedIndexes();
    policyConfig = new PolicyConfig(indexList, selectedPolicyId_, selectedHostId_, &nbQuery, &nb_.nbDBLock_);

    qDebug() << "test1";
    policyConfig->setModal(true);


    int result = policyConfig->exec();

    if (result == QDialog::Accepted) {
        setPolicyListFromDatabase();
        setPolicyTable();
        ui->tableWidget->clearSelection();
        nb_.updateHosts();
        nb_.block();
        qDebug() << "Policy Config Accecpted";
    }
}

void MainWindow::setPolicyListFromDatabase() {
    policyList_.clear();
    QSqlQuery nbQuery(nb_.nbDB_);

    {
        QMutexLocker ml(&nb_.nbDBLock_);
        nbQuery.prepare("SELECT policy_id, start_time, end_time, day_of_the_week, host_id FROM policy WHERE host_id = :host_id ORDER BY day_of_the_week ASC");
        nbQuery.bindValue(":host_id", selectedHostId_);
        nbQuery.exec();
    }

    while(nbQuery.next()) {
        QVector<QString> tmp;
        tmp.append(nbQuery.value(0).toString());
        tmp.append(nbQuery.value(1).toString());
        tmp.append(nbQuery.value(2).toString());
        tmp.append(nbQuery.value(3).toString());
        tmp.append(nbQuery.value(4).toString());

        PolicyObj tmpObj;
        tmpObj.set(tmp);

        policyList_.append(tmpObj);
    }
}

void MainWindow::resetPolicyTable() {
    for(int i = 0; i < ui->tableWidget->rowCount(); ++i) {
        for (int j = 0; j < ui->tableWidget->columnCount(); ++j) {
            ui->tableWidget->setItem(i, j, nullptr);
            ui->tableWidget->setColumnWidth(j, 100);
            ui->tableWidget->clearSpans();
        }
    }
}

void MainWindow::setItemPolicy(int row, int column, int policyId, int span) {
    if (span > 1) {
        ui->tableWidget->setSpan(row, column, span, 1);
    }

    QTableWidgetItem *itm = new QTableWidgetItem();
    itm->setBackground(Qt::gray);
    ui->tableWidget->setItem(row, column, itm);
    itm->setData(Qt::UserRole, policyId);
}

void MainWindow::setPolicyTable() {
    resetPolicyTable();
    for(QVector<PolicyObj>::iterator iter = policyList_.begin(); iter != policyList_.end(); ++iter) {
        int startHour = iter->getStartTime().leftRef(2).toInt();
        int startMin = iter->getStartTime().rightRef(2).toInt();
        int endHour = iter->getEndTime().leftRef(2).toInt();
        int endMin = iter->getEndTime().rightRef(2).toInt();

        int startRow = startHour + startMin / MINUTE::HOUR;
        int endRow = endHour + endMin / MINUTE::HOUR;
        int column = iter->getDayOfTheWeek();
        int span = 0;
        if (iter->getStartTime() <= iter->getEndTime()) {
            span = endRow - startRow;
            setItemPolicy(startRow, column, iter->getPolicyId(), span);
        } else {
            span = 24 - startRow;
            setItemPolicy(startRow, column, iter->getPolicyId(), span);

            span = endRow;
            setItemPolicy(0, column, iter->getPolicyId(), span);
        }
    }
}

void MainWindow::propLoad(QJsonObject jo) {
	jo["rect"] >> GJson::rect(this);
	jo["tableWidget"] >> GJson::columnSizes(ui->devTable);
}

void MainWindow::propSave(QJsonObject& jo) {
	jo["rect"] << GJson::rect(this);
	jo["tableWidget"] << GJson::columnSizes(ui->devTable);
}
