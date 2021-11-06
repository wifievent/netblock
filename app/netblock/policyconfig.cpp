#include "policyconfig.h"
#include "ui_policyconfig.h"

PolicyConfig::PolicyConfig(QModelIndexList indexList, int policyId, int hostId, QSqlQuery *query, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PolicyConfig)
{
    ui->setupUi(this);

    if(policyId == 0) {
        this->setWindowTitle("New");
    }
    else {
        this->setWindowTitle(QString::number(policyId));
    }

    hostId_ = hostId;
    policyId_ = policyId;

    query_ = query;

    QTime now = QTime::currentTime();
    sTime_ = QTime(now.hour(), now.minute() - now.minute() % 10);
    eTime_ = sTime_.addSecs(600);
    if(!policyId) {
        dayOfWeek_[indexList.constFirst().column()] = true;
        sTime_ = QTime(indexList.constFirst().row(), 0);
        eTime_ = QTime(indexList.constLast().row(), 0);
        if(indexList.constLast().row() + 1 == 24) {
            eTime_ = QTime(23, 50);
        }
    } else {
        ui->deleteButton->setEnabled(true);
        query_->prepare("SELECT host_id, start_time, end_time, day_of_the_week, FROM policy WHERE ploicy_id = :policy_id");
        query_->bindValue(":policy_id", policyId);
        query_->exec();
        query_->next();
        int sHour = query_->value(1).toString().leftRef(2).toInt();
        int sMin = query_->value(1).toString().rightRef(2).toInt();
        int eHour = query_->value(2).toString().leftRef(2).toInt();
        int eMin = query_->value(2).toString().rightRef(2).toInt();
        sTime_ = QTime(sHour, sMin);
        eTime_ = QTime(eHour, eMin);
        dayOfWeek_[query_->value(3).toInt()] = true;

        hostId_ = query->value(0).toInt();
    }

    qDebug() << ui->sHourBox->findText(QString::number(sTime_.hour()));

    ui->sHourBox->setCurrentIndex(ui->sHourBox->findText(QString::number(sTime_.hour())));
    qDebug() << "config1";
    ui->sMinBox->setCurrentIndex(ui->sMinBox->findText(QString::number(sTime_.minute())));
    qDebug() << "config1";
    ui->eHourBox->setCurrentIndex(ui->eHourBox->findText(QString::number(eTime_.hour())));
    qDebug() << "config1";
    ui->eMinBox->setCurrentIndex(ui->eMinBox->findText(QString::number(eTime_.minute())));
    qDebug() << "config1";
}

PolicyConfig::~PolicyConfig()
{
    delete ui;
}

void PolicyConfig::on_sHourBox_currentIndexChanged(int index)
{
    sTime_ = QTime(ui->sHourBox->currentText().toInt(), sTime_.minute());
    if(sTime_ > eTime_) {
        eTime_ = QTime(sTime_.hour() + 1, eTime_.minute());
    }
    setETimeLaterSTime();
}


void PolicyConfig::on_sMinBox_currentIndexChanged(int index)
{
    sTime_ = QTime(sTime_.hour(), ui->sMinBox->currentText().toInt());
    if(sTime_ > eTime_) {
        eTime_ = QTime(sTime_.hour() + 1, eTime_.minute());
    }
    setETimeLaterSTime();
}


void PolicyConfig::on_eHourBox_currentIndexChanged(int index)
{
    setETimeLaterSTime();
}

void PolicyConfig::setETimeLaterSTime() {
    ui->eHourBox->clear();
    for(int i = sTime_.minute() < 50 ? sTime_.hour() : sTime_.hour() + 1; i < 24; ++i) {
        ui->eHourBox->addItem(QString::number(i));
    }
    for(int i = sTime_.hour() == eTime_.hour() ? sTime_.minute() + 10 : 0; i < 60; i += 10) {
        ui->eMinBox->addItem(QString::number(i));
    }
    ui->eHourBox->setCurrentIndex(ui->eHourBox->findText(QString::number(eTime_.hour())));
    ui->eMinBox->setCurrentIndex(ui->eMinBox->findText(QString::number(eTime_.minute())));
}

void PolicyConfig::on_applyButton_clicked()
{
    dayOfWeek_[0] = ui->dayOfTheWeekCheck_0->isChecked();
    QList<QAbstractButton*> test = ui->dayOfWeekGroup->buttons();
    qDebug() << test.constLast()->objectName();
    close();
}

void PolicyConfig::on_deleteButton_clicked()
{
    query_->prepare("DELETE FROM policy WHERE policy_id = :policy_id");
    query_->bindValue(":policy_id", policyId_);
    query_->exec();
    close();
}

void PolicyConfig::on_cancelButton_clicked()
{
    close();
}
